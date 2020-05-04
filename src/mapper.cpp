#include <Arduino.h>
#include "mapper.h"

// ROMイメージをフラッシュ上に静的データとしてバンドルする
IMPORT_BIN(".rodata", "roms/hello.rom", ROM_IMAGE);
extern const uint8_t ROM_IMAGE[], _sizeof_ROM_IMAGE[];

// 未使用領域に割り当てる16KBのブランクイメージをバンドルする
IMPORT_BIN(".rodata", "roms/blank.rom", BLANK_IMAGE);
extern const uint8_t BLANK_IMAGE[], _sizeof_BLANK_IMAGE[];

// コナミマッパー(非SCC)をシミュレートしたメモリーマッパー処理
void mapper() {
  volatile uint8_t* bank[MAPPER_BANK_COUNT]; // ROMイメージのバンクアドレスマップ
  volatile uint8_t* mmap[MAPPER_RGST_COUNT]; // マッパーレジスター(未使用領域も含む)
  volatile uint8_t* dmmy[MAPPER_RGST_COUNT]; // 非マッパーROM用ダミーマッパー
  volatile uint32_t data = 0; // データバスアクセス用レジスターのプレースホルダー
  volatile uint32_t addr = 0; // アドレスバスアクセス用レジスターのプレースホルダー
  volatile uint32_t ofst = 0; // アドレスオフセット計算用レジスターのプレースホルダー
  volatile uint32_t iorq = 0; // 制御信号取得用レジスターのプレースホルダー

  // バンクアドレスマップの初期化処理
  int bank_count = (int)_sizeof_ROM_IMAGE / MAPPER_BANK_SIZE; // イメージサイズから実バンク数を算出
  if ((int)_sizeof_ROM_IMAGE > bank_count * MAPPER_BANK_SIZE) bank_count++; // 切り捨て補正
  bool entry_found = false;
  int entry_bank = 0; // ROMのエントリーポイントを含むバンク番号
  uint32_t entry_addr = 0; // ROM内のエントリーアドレス
  // ROMイメージをバンクサイズで分割したフラッシュ上の実アドレスを配列に割り付ける
  for (int i = 0; i < MAPPER_BANK_COUNT; i++) {
    if (i < bank_count) {
      // 実バンク数までは実アドレスを算出して割り付ける
      bank[i] = (uint8_t*)((uint32_t)ROM_IMAGE + MAPPER_BANK_SIZE * i);
      // 先頭がABで始まる最初に見つかったバンクをエントリーポイントとする
      if (entry_found) continue;
      if (bank[i][0] == 'A' && bank[i][1] == 'B') {
        entry_found = true;
        entry_bank = i;
        entry_addr = (uint32_t)bank[i][3] * 256 + bank[i][2];
      }
    } else {
      // 残りは未使用領域を割り付ける
      bank[i] = (uint8_t*)BLANK_IMAGE;
    }
  }

  // マッパーレジスターの初期化処理
  int mmap_begin = MAPPER_ADDR_BEGIN / MAPPER_BANK_SIZE; // 実際に使用可能な先頭の要素
  int mmap_end = MAPPER_ADDR_END / MAPPER_BANK_SIZE; // 実際に使用可能な末尾の要素
  int mmap_count = mmap_end - mmap_begin + 1; // 実際に使用可能なレジスターの要素数
  bool no_mapper = bank_count <= mmap_count; // 実バンク数が実レジスター数以下ならマッパーなし
  if (no_mapper && bank_count < mmap_count) {
    // 8000Hから開始する8〜16KBのROMも存在するので開始位置を調整する
    int page_space = (MAPPER_ADDR_END - MAPPER_ADDR_BEGIN + 1) / MAPPER_PAGE_SIZE; // マッパーの割り当てを持つページ数
    int page_count = (int)_sizeof_ROM_IMAGE / MAPPER_PAGE_SIZE; // イメージサイズから実ページ数を算出
    if ((int)_sizeof_ROM_IMAGE > page_count * MAPPER_PAGE_SIZE) page_count++; // 切り捨て補正
    if (page_count < page_space) {
      // エントリーアドレスを含むバンクがページ境界を跨ないように調整
      mmap_begin = (entry_addr / MAPPER_PAGE_SIZE) * (MAPPER_PAGE_SIZE / MAPPER_BANK_SIZE);
      mmap_end = mmap_begin + bank_count - 1;
    }
  }
  for (int i = 0, j = entry_bank; i < MAPPER_RGST_COUNT; i++) {
    if (i >= mmap_begin && i <= mmap_end && j < MAPPER_BANK_COUNT) {
      // バンクレジスターの実際に使用する範囲内の要素へ先頭から順にバンクの参照を割り付ける
      mmap[i] = bank[j++];
    } else {
      // 未使用レジスターは未使用領域を割り付ける
      mmap[i] = (uint8_t*)BLANK_IMAGE;
    }
  }

  // メインルーチン
  __disable_irq(); // 割込禁止
  asm volatile (
    "BEGIN:" "\n\t"
      "ldr %[iorq],%[idrA]" "\n\t"
    // メモリーアクセスの判定
    "MEM_REQ_TEST:" "\n\t"
      "tst %[iorq],%[MMSK]" "\n\t"
      "bne BEGIN" "\n\t" // メモリーアクセス発生までループ
    // メモリーアドレスの取得
      "ldr %[addr],%[idrB]" "\n\t"
      "ubfx %[ofst],%[addr],#0,%[MBMB]" "\n\t" // アドレスの下位ビットはオフセットとして
      "ubfx %[addr],%[addr],%[MBMB],%[MRBW]" "\n\t" // 上位ビットはマッパーレジスター指定として取得
    // メモリーリードライトの判定
    "MEM_RW_TEST:" "\n\t"
      "tst %[iorq],%[RMSK]" "\n\t"
      "bne MEM_WR_PROC" "\n\t" // リードでなければライトとみなす(ライト発生を待つと処理が間に合わない)
    // メモリーリード処理
    "MEM_RD_PROC:" "\n\t"
      "ldr %[addr],[%[mref],%[addr],lsl #2]" "\n\t" // マッパーレジスターアドレス＋レジスター指定×4バイト目の内容をバンクアドレスとして取得
      "ldrb %[data],[%[addr],%[ofst]]" "\n\t" // バンクアドレス＋オフセットの内容をROMデータとして取得
      "strb %[data],%[odrC]" "\n\t" // データをバスに準備する
      "strh %[OMSK],%[modC]" "\n\t" // 出力モードに切り替える
    "MEM_RD_WAIT:" "\n\t"
      "ldr %[iorq],%[idrA]" "\n\t"
      "tst %[iorq],%[MMSK]" "\n\t"
      "beq MEM_RD_WAIT" "\n\t" // メモリーアクセス終了までループ
      "strh %[IMSK],%[modC]" "\n\t" // 入力モードに切り替える
      "b BEGIN" "\n\t" // 先頭へ戻る
    // メモリーライト処理
    "MEM_WR_PROC:" "\n\t"
      // 書き込み可能範囲をチェックすると処理が間に合わない？ので省略
      // "cmp %[addr],%[MBGN]" "\n\t"
      // "blt MEM_WR_WAIT" "\n\t"
      // "cmp %[addr],%[MEND]" "\n\t"
      // "bgt MEM_WR_WAIT" "\n\t"
      "ldr %[data],%[idrC]" "\n\t"
      "ubfx %[data],%[data],#0,%[MBBW]" "\n\t" // マッパーレジスターへのバンク指定を取得
      "ldr %[data],[%[bank],%[data],lsl #2]" "\n\t" // バンク配列のアドレス＋バンク指定×4バイト目の内容をバンクアドレスとして取得
      "str %[data],[%[mreg],%[addr],lsl #2]" "\n\t" // マッパーレジスターアドレス＋レジスター指定×4バイト目の内容を書き換える
    "MEM_WR_WAIT:" "\n\t"
      "ldr %[iorq],%[idrA]" "\n\t"
      "tst %[iorq],%[MMSK]" "\n\t"
      "beq MEM_WR_WAIT" "\n\t" // メモリーアクセス終了までループ
      "b BEGIN" "\n\t" // 先頭へ戻る
    "END:" "\n\t" // 上で無限ループしているので処理を抜けることはない
    :
    : [clrA]"m"(portA->BRR), // ポートA(制御信号)のビットリセットレジスター
      [setA]"m"(portA->BSRR), // ポートA(制御信号)のビットセットレジスター
      [idrA]"m"(portA->IDR), // ポートA(制御信号)からの入力
      [idrB]"m"(portB->IDR), // ポートB(アドレスバス)からの入力
      [idrC]"m"(portC->IDR), // ポートC(データバス)からの入力
      [odrC]"m"(portC->ODR), // ポートC(データバス)への出力
      [modC]"m"(portC->MODER), // ポートC(データバス)の入出力モードレジスター
      [MMSK]"I"(SLTSL_MASK | MERQ_MASK),
      [RMSK]"I"(RD_MASK),
      [WMSK]"I"(WR_MASK), // ライト信号はタイミングがシビアなので未使用
      [IMSK]"r"(CLEAR_MODE_LSB_MASK),
      [OMSK]"r"(OUTPUT_MODE_LSB_MASK),
      [MBGN]"I"(MAPPER_RGST_BEGIN),
      [MEND]"I"(MAPPER_RGST_END),
      [MBMB]"I"(MAPPER_BANK_MSB),
      [MRBW]"I"(MAPPER_RGST_WIDTH),
      [MBBW]"I"(MAPPER_BANK_WIDTH),
      [bank]"r"(bank),
      [mref]"r"(mmap),
      [mreg]"r"(no_mapper ? dmmy : mmap), // 非マッパーROMでは書き込み先としてダミーマッパーを渡す
      [data]"r"(data),
      [addr]"r"(addr),
      [ofst]"r"(ofst),
      [iorq]"r"(iorq)
  );
  __enable_irq(); // 割込許可
  // 処理を抜けてしまうとmain.cppのloop()から再度呼び出されてマッパー初期化から再実行となってしまうので注意
}
