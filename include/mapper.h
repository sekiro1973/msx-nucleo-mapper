#ifndef MAPPER_H
#define MAPPER_H

#include "tools.h"

#define CLEAR_MODE_LSB_MASK 0xffff0000 // 入力モード切り替えの設定値
#define OUTPUT_MODE_LSB_MASK 0x5555 // 出力モード切り替えの設定値

#define WAIT_MASK (1 << 0)
#define INT_MASK (1 << 1)
#define BUSDIR_MASK (1 << 4)
#define LED_MASK (1 << 5)

#define WR_MASK (1 << 8)
#define RD_MASK (1 << 9)
#define MERQ_MASK (1 << 10)
#define IORQ_MASK (1 << 11)
#define SLTSL_MASK (1 << 12)

#define MAPEER_ADDR_MSB 16 // メモリー(64KB)の最上位ビット番号
#define MAPPER_ADDR_SIZE (1 << MAPEER_ADDR_MSB) // メモリーの容量
#define MAPPER_ADDR_BEGIN 0x4000 // マッパー割り当て範囲の開始アドレス
#define MAPPER_ADDR_END 0xbfff // マッパー割り当て範囲の終了アドレス

#define MAPPER_PAGE_MSB 14 // ページ内の最上位ビット番号
#define MAPPER_PAGE_WIDTH (MAPEER_ADDR_MSB - MAPPER_PAGE_MSB) // ページ番号指定ビット幅
#define MAPPER_PAGE_SIZE (1 << MAPPER_PAGE_MSB) // ページの容量
#define MAPPER_PAGE_COUNT (1 << MAPPER_PAGE_WIDTH) // 指定可能ページ数

#define MAPPER_BANK_MSB 13 // マッパー範囲内のアドレス最上位ビット番号
#define MAPPER_BANK_WIDTH 5 // バンク番号指定ビット幅
#define MAPPER_BANK_SIZE (1 << MAPPER_BANK_MSB) // バンクの容量
#define MAPPER_BANK_COUNT (1 << MAPPER_BANK_WIDTH) // 指定可能バンク数

#define MAPPER_RGST_WIDTH (MAPEER_ADDR_MSB - MAPPER_BANK_MSB) // マッパーレジスター要素番号指定ビット幅
#define MAPPER_RGST_COUNT (1 << MAPPER_RGST_WIDTH) // 指定可能レジスター数
#define MAPPER_RGST_BEGIN 3 // マッパーレジスターの先頭要素番号
#define MAPPER_RGST_END 5 // マッパーレジスターの末尾要素番号

#endif

extern GPIO_TypeDef* portA;
extern GPIO_TypeDef* portB;
extern GPIO_TypeDef* portC;

void mapper();
