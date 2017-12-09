# arduino
このディレクトリには、Arduino サブシステムのスケッチ等が含まれます。

## sketch

### 概要

メイン CPU と /dev/ttyAMA0 を介したシリアル通信により、センサーデータの集約や LED リングの制御を行う Arduino サブシステムのスケッチ。

### ビルド

Tumbler にプリインストールされている Arduino 開発環境が前提となります。

``````````{.cpp}
$ make
$ make upload reset_stty
``````````

これにより、スケッチがビルドされ Arduino に転送されて自動的に起動します。
