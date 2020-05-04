# msx-nucleo-mapper

## 概要

[STM32 Nucleo開発ボード](https://www.stmcu.jp/design/hwdevelop/nucleo/)により、MSXのROMカートリッジのシミュレーターを作るサンプルコードです。

CやC++を使い慣れておらず読みづらいコードになってしまったかもしれませんが、何卒ご容赦ください。


## 動作環境

以下の環境で動作を確認しました。

* STM32 Nucleo-64
  * Nucleo-L476RG (STM32L476 80MHz・Flash 1MB・RAM 128KB)

* MSX
  * Terasic DE0-CV + DEOCM
  * Canon V-8
  * Panasonic FS-A1ST


## 実行方法

以下の手順で実行してください。

1. プロジェクトをダウンロードするかGitでクローンしてPC上の適当な場所に展開します。
    * Gitほか、開発ツール類の使用方法は配布元ホームページなどを参照してください。
2. PCに[Microsoft Visual Studio Code](https://code.visualstudio.com)をインストールして起動します。
    * 作者はmacOS Mojaveを使用しましたが、WindowsやLinuxでも動作すると思います。
3. サイドバーの拡張機能から[PlatformIO IDE](https://platformio.org/platformio-ide)を検索してインストールします。
    * サイドバーにPlatformIOのアイコンが現れるようになります。
4. PlatformIOのProjects & ConfigurationでAdd Existingから、手順1のプロジェクトフォルダーを開きます。
    * このメニューからプロジェクトで使用するボードの種類などの設定を変更できます。
5. PCとNucleoボードをUSBで接続し、PlatformIOのメニューからビルドしてアップロードします。
    * ビルドやアップロードでエラーが発生していないか特に注意してください。
5. PCから取り外したNucleoボードを電源を切ったMSXに下図のようにして接続します。
    * 配線を間違えると誤動作したりショートしてNucleoやMSXを破壊する危険があります。
6. MSXの電源を入れると、通常のROMカートリッジのようにボードのROMイメージが起動します。
    * ボードのROMイメージは何度でもアップロードして好きな内容に差し替えられます。

![配線図](https://github.com/sekiro1973/msx-nucleo-mapper/blob/master/layout.png)


## 解説

フレームワークにArduinoを使用しているのはボードの初期化やシリアルモニターでのデバッグを簡易にしたかったためで、メインであるマッパー処理の動作にはフレームワークの機能は使用していません。

Nucleoボードには外見がほぼ同一でCPUなどの仕様が少しずつ異なる機種が多数存在します。本プロジェクトではNucleo-L476RGを使用しましたが、未確認ながらピン配置が同一で十分高速(72MHz以上？)な機種なら使用ボードの設定を変更するだけで動作する可能性が高いと考えられます。

[STM32 Nucleo-64 Boards User Manual (UM1724).pdf](https://www.stmcu.jp/design/document/users_manual/52239/)によると、Nucleo-F4シリーズにはPB11の割り当てがないためそのままでは使用できません。原理的には他のピンへの振り替えは可能ですが、F446REのような高クロック(180MHz)機でないと難しそうです。また、ボード裏面のチップ抵抗の変更によりPC13(プッシュボタンB1)およびPC14〜15(外部クロック入出力)を不使用にする場合、PB0〜7をデータバスに、PC0〜15をアドレスバスに変更することで動作しそうです。

Nucleoボードへのアップロードプロトコルにはstlinkではなくmbedを使用しています。stlinkでは32KBまでしかアップロードできず、バンドルしているブランクイメージだけで一杯になってしまうためです。おそらくオープンソースの無償版での制約によるものと思いますが詳細は調べていません。mbedではこの制限はありませんでした。

src/roms/hello.romは[z88dk](https://www.z88dk.org/forum/)でビルドしたMSX用ROMイメージで、起動すると「hello, world」とだけ表示します。ROMイメージにはマッパー不使用の8〜32KBのもの、非SCCのコナミマッパーを使用しているものを指定できるはずですが、該当する全てのROMイメージをテストすることはできないので、動作しないものもあるかもしれません。

マッパー関連の配列処理は、インラインアセンブラによるメイン処理内でのアドレス計算を最小限にするためのものです。このほか、メイン処理ではARMのオフセット・シフト付きアドレス指定やバイト単位・ハーフワード単位のロード・ストアを使用することで命令数を削減しています。なお、メイン処理ではMSXによるROMのリード中にBUSDIRをLにすることは省略しています。そのため拡張スロットでは誤動作する場合があるかもしれません。


## 雑感

最初、Arduinoで試行錯誤したのですが、16MHzのAVRでは相手が非力な3.58MHzのZ80であってもI/Oポートの入出力を扱うのが精一杯でした。その際、初めて今時のCPUのインラインアセンブラを書いたのですが、同じ8ビットでもZ80のようなテクニックを駆使する場面もなくあっさり動作してしまい拍子抜けしました。結局、ArduinoではI/Oアクセスにちょっとしたメモリーアクセス処理を追加しただけでも追従できなくなってしまったため、代わりになるマイコンを探したところ、32ビットのARMを積んだNucleoが5V入力を扱える上に大変安価なことを知って飛び付いた次第です。しかしながら80MHzでも予想ほどには余裕がなく、コンパイラーが出力したコードを手作業で最適化する作業はハンドアセンブルしていた少年時代が思い出されて楽しかったです。


## ライセンス

include/tools.hで「[ELM by ChaN](http://elm-chan.org/)」様配布プログラム(mary_sdc.zip/LPC1100.h)内のIMPORT_BINマクロを引用しました。パブリックドメインとの記載でしたが念のためここに報告します。また、これに倣って本プロジェクトのinclude/tools.hもパブリックドメインとします。

あなたが下記に同意できる場合に限り、本プロジェクトの成果物を著作権表示なしで自由に実行・改変・再配布しても構いません。

* 本プロジェクトは無保証です。
* 本プロジェクトを違法行為に使用しないでください。
* 作者はあなたが損害を被っても責任を負いません。
* 作者はリクエストや連絡に応える義務はありません。
* 作者の著作権を勝手に行使しないでください。


## 作者

@sekiro1973
