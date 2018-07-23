# arduino
このディレクトリには、Arduino サブシステムのスケッチ等が含まれます。

## Arduino 規格

- プログラム容量   30KB
- データ容量        2KB
- EEPROM容量        1KB
- シリアル通信  19200bps

## sketch

### 概要

メイン CPU と /dev/ttyAMA0 を介したシリアル通信により、センサーデータの集約や LED リングの制御を行う Arduino サブシステムのスケッチです。それぞれのファイルは以下のような内容となります。Controller.ino が全体の制御ループであり、その中から、各センサー等を処理するためのインスタンスが呼ばれます。

|ファイル名|説明|
|---|---|
|Controller.ino|Ardionoサブシステムのメイン処理|
|Module.h|各種装置制御の親クラス|
|LEDRing.h|LEDリングのONOFF制御|
|BMEControl.h|環境センサーの計測処理|
|TouchSensor.h|タッチセンサーの状態制御|
|EEPROMTable.h|EEPROMの番地管理|
|EEPROMControl.h|EEPROMの制御|
|SerialIO.cpp/h|シリアル受信処理|

### ビルド

Tumbler にプリインストールされている Arduino 開発環境が前提となります。

``````````{.cpp}
$ cd sketch
$ make
$ make upload reset_stty
``````````

これにより、スケッチがビルドされ Arduino に転送されて自動的に起動します。`reset.sh` は通信経路をリセットするスクリプトであり、通常は利用する必要はありません（Tumbler 起動時にリセットする必要がありますが、そのリセットは `/etc/rc.local` スクリプトにより実行されています）

