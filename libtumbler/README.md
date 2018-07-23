# libtumbler
Hardware contol library for Fairy I/O Tumbler

## 概要

libtumbler は、Tumbler の各サブシステムを制御するためのライブラリです。下記のサブシステムが制御対象となります。

### 公開済

- LED リング
- スピーカー制御（簡易版）
- タッチセンサ
- 環境センサ

### 未公開

- 照度センサー

## 構築

### 依存ライブラリ

#### 必須

- Wiring Pi（wiringpi）
- ALSA library（libasound2-dev）

## ビルド

``````````{.cpp}
$ autoreconf -vif
$ ./configure
$ make
$ make check 
$ make install
``````````

- `make check` はビルド自体には不要です。`make check` では LED リングが点灯したり、音声が再生されたりする場合がありますのでご注意ください。
- `make chcek` が動作しない場合、Arduino スケッチが古い可能性があります。本レポジトリの `arduino/` 以下から最新スケッチをインストールしてください。
- Tumbler 上でのビルド及び動作のみ確認されています。

## サンプルプログラム

未公開ですが、`test/` ディレクトリのテストプログラムが参考になります。

## API 簡易チュートリアル

### タッチセンサー

#### 1. タッチセンサー用コールバック関数を定義する

Arduinoはlibtumblerにタッチセンサーの状態が変化したタイミングで通知を送ります。<BR>
これに伴い、通知を受けた時に呼び出されるコールバック関数を定義します。

``````````.cpp
static void callbackfnc(
  char* buffer,  // 受信したデータの先頭ポインタ
  int len)       // 上記受信データの長さ
```````````

作成したコールバック関数をシリアル受信用スレッドに登録します。

``````````.cpp
TouchSensor& sensor = TouchSensor::getInstance();
sensor.setCallBackFnc(callbackfnc);
```````````

受信データは先頭2文字を確認することでそのデータ種別が確認できます。

|#|先頭|フォーマット|説明|
|---|---|---|---|
|1|TS |TS\* \* \* \* |タッチセンサーのONOFF状態(後述)|
|2|TT |TT\*\* \*\* \*\* \*\* |タッチセンサーの設定感度|
|3|TV |TV\*\* \*\* \*\* \*\* |タッチセンサーの計測値|

なお、送られて来た各種データはTouchSensorクラスのメンバとしても取得されています。タッチセンサーのONOFF状態は以下のように表現されています。

|#|状態|説明|
|---|---|---|
|1|0 |OFF|
|2|1 |OFF → 短押し|
|3|2 |短押し|
|4|3 |短押し → 長押し|
|5|4 |長押し|
|6|5 |短押し → OFF|
|7|7 |長押し → OFF|

長押しは約1秒間タッチし続けることで切り替わります。

#### 2. タッチセンサーの設定を行う

タッチセンサーの感度はArduino内臓のEEPROMにて記憶しています(デフォルト50)。お使いの環境に合わせて任意の値を設定します(推奨値：20～100)。

``````````.cpp
TouchSensor& sensor = TouchSensor::getInstance();
sensor.setThreshVal(false,50,50,50,50,true); // 値指定,EEPROMに記録する
sensor.setThreshPer(false,0.5,0.5,0.4,0.6,false); // 割合指定(0.0:100 - 1.0:20)
```````````

タッチセンサー状態の通知機能のONOFFを設定します(起動時ON)。

``````````.cpp
TouchSensor& sensor = TouchSensor::getInstance();
sensor.setReport(false,0);  // OFF
```````````

