#include <Arduino.h>
#include "mapper.h"

GPIO_TypeDef* portA;
GPIO_TypeDef* portB;
GPIO_TypeDef* portC;

// void pause() {
//   // ポートCの13ピン(プッシュボタン)の状態を判定する
//   if (portC->IDR & 0x2000) {
//     portA->BSRR = INT_MASK; // INTをLに
//   } else {
//     portA->BRR = INT_MASK; // INTをHに
//   }
// }

void loop() {
  mapper();
}

void setup() {
  // Serial.begin(9600);
  // Serial.print("\n");

  portA = get_GPIO_Port(0);
  portB = get_GPIO_Port(1);
  portC = get_GPIO_Port(2);

  // ポートBの16ピンをフルに使えるのでアドレスバスを割り当てる
  pinMode(D49, INPUT); // A0 -> PB0
  pinMode(D41, INPUT); // A1 -> PB1
  pinMode(D40, INPUT); // A2 -> PB2
  pinMode(D3 , INPUT); // A3 -> PB3
  pinMode(D5 , INPUT); // A4 -> PB4
  pinMode(D4 , INPUT); // A5 -> PB5
  pinMode(D10, INPUT); // A6 -> PB6
  pinMode(D22, INPUT); // A7 -> PB7
  pinMode(D15, INPUT); // A8 -> PB8
  pinMode(D14, INPUT); // A9 -> PB9
  pinMode(D6 , INPUT); // A10 -> PB10
  pinMode(D39, INPUT); // A11 -> PB11
  pinMode(D38, INPUT); // A12 -> PB12
  pinMode(D44, INPUT); // A13 -> PB13
  pinMode(D43, INPUT); // A14 -> PB14
  pinMode(D42, INPUT); // A15 -> PB15

  // ポートCは13〜15ピンを避けて下位8ピンにデータバスを割り当てる
  pinMode(D51, INPUT); // D0 -> PC0
  pinMode(D50, INPUT); // D1 -> PC1
  pinMode(D28, INPUT); // D2 -> PC2
  pinMode(D29, INPUT); // D3 -> PC3
  pinMode(D45, INPUT); // D4 -> PC4
  pinMode(D35, INPUT); // D5 -> PC5
  pinMode(D34, INPUT); // D6 -> PC6
  pinMode(D9 , INPUT); // D7 -> PC7

  // ポートAは2〜3・5・13〜14ピンを避けて制御信号の入出力を割り当てる
  // pinMode(D46, OUTPUT_OPEN_DRAIN); // WAIT -> PA0
  pinMode(D47, OUTPUT_OPEN_DRAIN); // INT -> PA1
  // PA2 ※USART2が使うので使用不可
  // PA3 ※USART2が使うので使用不可
  pinMode(D48, OUTPUT); // BUSDIR -> PA4
  pinMode(D13, OUTPUT); // LED -> PA5
  // pinMode(D12, INPUT); // NC -> PA6
  // pinMode(D11, INPUT); // NC -> PA7
  pinMode(D7 , INPUT); // WR -> PA8
  pinMode(D8 , INPUT); // RD -> PA9
  pinMode(D2 , INPUT); // MERQ -> PA10
  pinMode(D37, INPUT); // IORQ -> PA11
  pinMode(D36, INPUT); // SLTSL -> PA12
  // PA13 ※ST-LINKが使うので使用不可
  // PA14 ※ST-LINKが使うので使用不可
  // pinMode(D21, OUTPUT); // NC -> PA15

  // digitalWrite(D46, HIGH);
  digitalWrite(D47, HIGH);
  digitalWrite(D48, HIGH);
  digitalWrite(D13, LOW);

  // ボード上のプッシュボタンによる割込の実験の残骸
  // attachInterrupt(D21, pause, CHANGE);
}
