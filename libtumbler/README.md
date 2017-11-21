# libtumbler
Hardware module contol library for Fairy I/O Tumbler

## 概要

libtumbler は、Tumbler の各サブシステムを制御するためのライブラリです。下記のサブシステムが制御対象となります。

- LED リング（公開済）
- タッチセンサ（未公開）
- 環境センサ（未公開）

## ビルド

``````````{.cpp}
$ autoreconf -vif
$ ./configure --prefix=(任意のインストール先)
$ make
$ make check 
$ make install
``````````

- `make check` はビルド自体には不要です。`make check` では LED リングが点灯したり、音声が再生されたりしますのでご注意ください。
- `make chcek` が動作しない場合、Arduino スケッチが古い可能性があります。本レポジトリの `arduino/` 以下から最新スケッチをインストールしてください。

## サンプルプログラム

未公開
