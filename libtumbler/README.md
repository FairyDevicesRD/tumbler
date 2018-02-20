# libtumbler
Hardware contol library for Fairy I/O Tumbler

## 概要

libtumbler は、Tumbler の各サブシステムを制御するためのライブラリです。下記のサブシステムが制御対象となります。

### 公開済

- LED リング
- スピーカー制御（簡易版）

### 未公開

- タッチセンサ
- 環境センサ

## 構築

### 依存ライブラリ

#### 必須

- Wiring Pi (wiringpi) 2.4 以上
- ALSA (libasound2-dev) 1.0 以上

### ビルド

``````````{.cpp}
$ autoreconf -vif
$ ./configure
$ make
$ make check 
$ make install
``````````

- `make check` はビルド自体には不要です。`make check` では LED リングが点灯したり、音声が再生されたりする場合がありますのでご注意ください。
- `make chcek` が動作しない場合、Arduino スケッチが古い可能性があります。本レポジトリの `arduino/` 以下から最新スケッチをインストールしてください。

## サンプルプログラム

未公開
