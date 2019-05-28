# libtumbler
Hardware contol library for Fairy I/O Tumbler

## 概要

libtumbler は、Tumbler の各サブシステムを制御するためのライブラリです。下記のサブシステムが制御対象となります。

### 機能

#### デフォルトで有効な機能

- LED リングの制御
- トップパネル上の４つのタッチボタンの接触状態の取得
- 簡易スピーカー制御（通常のスピーカー制御には portaudio 等の一般的なライブラリをご利用ください）

#### オプションで有効化する機能

- 環境センサー（BME280）からの温度、湿度、気圧データの取得（環境設定上の留意点がありますので、下記を御覧ください）
- 光センサー（LTR-329ALS）からの周辺照度データの取得（環境設定上の留意点がありますので、下記を御覧ください）
- 赤外線 I/O 機能（正面近接センサー、外部赤外線信号受信）

## 構築

### 依存ライブラリ

#### 必須

- Wiring Pi（wiringpi）
- ALSA library（libasound2-dev）

#### オプション

##### 赤外線 I/O を利用する場合に必要

- [pigpio library](http://abyz.me.uk/rpi/pigpio/)

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
- Tumbler 上でのビルド及び動作のみ確認されています。

### configure オプション

|オプション|値|内容|初期値|
|---|---|---|---|
|--enable-debug|-|オプションを指定することで、デバッグビルドが有効になります|無効|
|--enable-envsensor|-|オプションを指定することで、環境センサーサポートが有効化されます|無効|
|--enable-lightsensor|-|オプションを指定することで、光センサーサポートが有効化されます|無効|
|--enable-irio|-|オプションを指定することで、赤外線 I/O サポートが有効化されます|無効|
|--with-pigpio|pigpio library への絶対パス|pigpio ライブラリをビルドしたディレクトリを指定します|なし|

#### 環境センサー及び光センサーサポートを有効化する場合の環境設定上の留意点

環境センサー及び光センサーはそれぞれデフォルトでは無効化されています（OS での I2C サポートが無効化されています）。環境センサーもしくは光センサーを有効にする場合、Tumbler の I2C サポートを有効にしておく必要があります。このために、`sudo raspi-config` コマンドを用いて、`5 Interfacing Options` を選択し、`P5 I2C` を選択し、I2C インターフェースを有効化してください。

## libtumbler API

### API の概要

各サブシステムは、ライブラリ上はシングルトン・インスタンスとして取得することができます。シングルトン・インスタンスに対するメンバー関数呼び出しにより、各サブシステムをコントロールします。

### API の利用例の一覧

|サンプルプログラム|内容|
|---|---|
| [examples/ledring.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/ledring.cpp) |LED リングの色、位置、組込アニメーション等を制御する簡単な利用例|
|[examples/ledring2.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/ledring2.cpp) |LED リングの外部制御アニメーションに関する利用例|
|[examples/buttons.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/buttons.cpp)|タッチボタンの利用例|
|[examples/buttons2.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/buttons2.cpp)|タッチボタンの利用例に長押しの判定機能を追加した例|
|[examples/buttons3.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/buttons3.cpp)|マルチタッチを禁止したタッチボタンの利用例|
|[examples/buttons4.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/buttons3.cpp)|短押し、長押しを交えたタッチボタンによるアプリケーションの例|
|[examples/envsensor.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/envsensor.cpp)|環境センサーの利用例|
|[examples/lightsensor.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/buttons3.cpp)|光センサーの利用例|
|[examples/irproximitysensor.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/irproximitysensor.cpp)|赤外線 I/O による正面近接センサーの利用例|
|[examples/irsignalreceiver.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/irsignalreceiver.cpp)|赤外線 I/O による外部赤外線信号受信の利用例|
|[examples/irall.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/irall.cpp)|赤外線 I/O による正面近接センサーと、外部赤外線信号受信の同時利用例|

### LED リング制御

#### LED クラス

LED クラスは、ひとつのフルカラー LED を表すクラスです。ひとつのフルカラー LED は、赤・緑・青、それぞれ 256 段階の値を取ることができます。コンストラクタで RGB を指定してインスタンス化することができます。

サンプルプログラムは `/examples` 以下にあります。


##### LED クラスのコンストラクタ

``````````.cpp
LED::LED();
LED::LED(int r,int g,int b);
``````````

コンストラクタでは、R,G,B をそれぞれ [0,255] で指定することができます。指定しなかった場合は、全てゼロ、すなわち消灯状態となります。

#### Frame クラス

Frame クラスは、LED リングの全体、すなわち 18 個の色指定された LED インスタンスを持つクラスです。フレームの名称は、アニメーションする場合のキーフレームとなることに由来しています。

##### Frame クラスのコンストラクタ

``````````.cpp
Frame::Frame();
Frame::Frame(const LED& background);
``````````

デフォルトコンストラクタを利用した場合、18 個すべての LED は消灯状態となります。オーバーロードされたもうひとつのコンストラクタでは、ひとつの LED インスタンスを取ることができ、18 個すべての LED は指定された LED の色指定となります。

##### setLED()

``````````.cpp
void Frame::setLED(int index, const LED& led);
``````````

第一引数で指定した位置に、第二引数で指定した LED をセットします。`index` は LED 番号であり、[0,17] の値を取ります。

#### LEDRing クラス

LEDRing クラスは、Tumbler の LED リングを表すクラスです。シングルトン・インスタンスとして、複数のスレッドから利用することが可能です。LEDRing クラスの点灯方式は２つあります。ひとつは内部制御点灯です。内部制御点灯とは、Arduino スケッチに組み込まれたパターンでのアニメーションを行う点灯方式のことです。フレームは固定となり、アニメーションパターンも限られますが、簡単に点灯させることが可能で、パターンが内部に組み込まれていることから、libtumbler の制御を離れた後にもアニメーションを継続することが可能です。

もうひとつは外部制御点灯です。外部制御点灯とは、libtumbler から通信によるリアルタイム制御でアニメーションを行う点灯方式のことです。フレームや動き方が変化するような複雑なアニメーションパターンを実行させることができますが、libtumbler の制御を離れた後に、当該パターンを維持することはできません。

##### インスタンスの取得

`LEDRing& ring = LEDRing::getInsntance()` として LED リングのインスタンスを取得します。引数はありません。

##### reset()

``````````.cpp
int LEDRing::reset(bool async)
``````````

LED リングをリセットし全消灯します。第一引数ではリセット命令の終了を待たない（非同期的に実行する）かどうかを指定します。通常は待つ必要がないため、`true` を指定することができます。

##### motion()

``````````.cpp
int LEDRing::motion(bool async, uint8_t animationPattern, const Frame& frame);
``````````

内部制御点灯を行います。第一引数では、内部制御点灯命令の完了を待たない（非同期的に実行する）かどうかを指定します。通常は待つ必要はないため、`true` を指定することができます。第二引数は、内部に組み込まれたアニメーションパターンを指定します。0 の場合、アニメーションしません（停止）、1 の場合、時計回りに回転します。2 の場合、反時計回りに回転します。第三引数では、アニメーションに用いるフレームを１つ指定します。内部制御点灯ではアニメーション中にフレームを変更することはできません。この関数が呼ばれた直後 LED リングは一度リセットされます。

アニメーションパターンは内部に組み込まれているため、この関数が呼ばれた後は、libtumbler の制御を離れても、同じアニメーションパターンでの点灯が継続されます。

##### addFrame() / setFrames() 

``````````.cpp
void LEDRing::addFrame(const Frame& frame);
void LEDRing::setFrames(const std::vector<Frame>& frames);
``````````

外部制御点灯では、追加した複数フレームを、指定した FPS で順に点灯させるという処理を行います。この関数では、LEDRing クラスに対して、フレームを追加または複数フレームをまとめてセットし、準備します。

##### clearFrames()

``````````.cpp
void LEDRing::clearFrames();
``````````

登録されたフレームを削除します。フレームを削除するだけでは LED リングは消灯しません。LED リングを消灯したいときは、`reset()` 関数を用いるか、明示的に消灯された LED をセットしたフレームを `show()` してください。

##### setFPS()

``````````.cpp
void LEDRing::setFPS(int fps);
``````````

外部制御点灯では、追加した複数フレームを、指定した FPS で順に点灯させるという処理を行います。この関数では、LEDRing クラスに登録された複数フレームを何FPSで表示するかを指定します。FPS（Frame Per Seconds）は、1 秒間に表示されるフレーム数を表します。10 FPS の場合、1 秒間に 10 フレームが表示されることになります。ただし、セットされた FPS は要求 FPS であり、大きすぎる FPS は再現されません。FPS のデフォルト値は 1 です。この関数は、内部制御点灯には影響を与えません。

##### show()

``````````.cpp
void LEDRing::show(bool async);
``````````

addFrame() / setFrames(), setFPS() が呼ばれた後に、実際に外部制御点灯を実行します。第一引数では、外部制御点灯命令の完了を待たない（非同期的に実行する）かどうかを指定します。この関数は、登録されたフレームをすべて表示した後に終了します。すなわち、同期的に呼ばれた `show()` 関数は、登録されたすべてのフレームを表示し終わるまでブロックされます。再生終了後、LED リングは、登録されたフレーム群の最後のフレームが固定表示され続けます。この固定表示状態は、LED リング内部で処理されているため、libtumbler のプロセスの存在に関わらず、常に表示され続けることに留意してください。

典型的な利用事例として、発話開始イベントの発生時に、登録されたアニメーションフレームを非同期で再生開始し、発話終了イベントの発生時に、LED リングをクリアする（もしくは何らかのアニメーションパターンを内部制御点灯で非同期で再生開始する）等があります。この利用例については、[examples/ledring2.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/ledring2.cpp) を参考にすることができます。

### タッチボタン制御

#### Buttons クラス

Buttons クラスは、Tumbler の上面パネルに設置された４つのタッチボタン群を表すクラスです。シングルトン・インスタンスとして、複数のスレッドから利用することが可能です。Buttons クラスは、デフォルトではマルチタッチを許可していますが、設定によりシングルタッチのみに強制することができます。ボタン間の距離が近いため、誤って隣のボタンに触れてしまう場合があり、このような場合の誤動作を防ぐために、シングルタッチの強制は役に立つ場合があります。ボタンへのタッチが検出された場合、ユーザー定義コールバック関数が呼ばれます。

サンプルプログラムは `/examples` 以下にあります。

##### インスタンスの取得

``````````.cpp
Buttons& button = Buttons::getInsntance(ButtonStateCallback func, void* userdata); // (1)
Buttons& button = Buttons::getInsntance(ButtonStateCallback func, const ButtonDetectionConfig &config, void* userdata); // (2)
``````````

以上のように Button クラスのインスタンスを取得することができます。第一引数には、ボタンがタッチされたときに呼ばれるコールバック関数を指定します。(2) のオーバーロード関数では、第二引数にボタン検出の設定を与えます。第三引数には、コールバック関数に渡される任意のデータを指定することができます。

##### start()

``````````.cpp
void Buttons::start()
``````````

タッチボタンの検出を開始します。

##### stop()

``````````.cpp
void Buttons::stop()
``````````

タッチボタンの検出を終了します。

#### ButtonState 型

``````````.cpp
enum class DLL_PUBLIC ButtonState
{
	none_,    //!< 何も検知されていない状態
	pushed_,  //!< 指の接触が検知された状態
	released_,//!< 接触検知状態から、指が離された状態（pushed_ ステートからの変化として 1 回のみ呼ばれ、以降は none_ ステートになる）
};
``````````

`ButtonState` 型は、タッチボタンの検出状態を表す列挙型です。それぞれの値の意味は上記の通りとなりますが、留意点として、`ButtonState::released_` ステートは、`ButtonState::pushed_` ステートからの変化として、指が離されたときに 1 回のみ出現することに留意してください。すなわちステートは、`... -> none_ -> none_ -> pushed_ -> pushed -> ... -> pushed_ -> released_ -> none_ -> none -> ...` のように変化します。

タッチされたことを示す状態は `ButtonState::pushed_` ステートのみです。例えば長押し状態等の判定は、`ButtonState::pushed_` が一定時間連続して発生することによって判定可能であり、長押し状態の判定については、[examples/buttons2.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/buttons2.cpp)を参考にすることができます。

#### ButtonInfo クラス

``````````.cpp
class DLL_PUBLIC ButtonInfo
{
public:
	std::vector<int> baselines_;  //!< 各タッチボタンのベースライン補正値（非接触状態の測定値）
	std::vector<int> corrValues_; //!< ベースライン補正値を減算した各タッチボタンの補正済計測値
};
``````````

`ButtonInfo` クラスは、接触状態の測定結果を保有するデータクラスです。通常は利用することはありません。

#### ButtonDetectionConfig クラス


``````````.cpp
class DLL_PUBLIC ButtonDetectionConfig
{
public:
	bool multiTouchDetectionEnabled_ = true; //!< マルチタッチを有効にする（マルチタッチ無効の場合は、先に押されたボタンのみ有効。完全同時に押された場合は、より強く押された方のみ有効となる）
	bool manualThreshold_ = false; //!< 補正済計測値からの増分閾値を以下に指定する指定値にする
	int manualThresholdValues_[4]; //!< 増分閾値の指定
};
``````````

タッチボタンの検出設定を行う設定データクラスです。`multiTouchDetectionEnabled_` によりマルチタッチの有効／無効を設定することができます。デフォルトでは、上記の通り、マルチタッチが有効になっています。マルチタッチ無効の場合に、複数ボタンを同時にタッチした場合は、先にタッチされた側のボタンのみ有効となります。完全同時だった場合は、より強くタッチされた側のボタンのみ有効となります。


`manualThreshold_` は、タッチセンサーの検出閾値をユーザー指定値にすることができる設定項目であり、ユーザー指定値については `manualThresholdValues_` に指定を行いますが、通常は利用することはありません。特段の事情により、タッチセンサーの感度を強制的に下げる／上げる場合には、30 〜 40 を中心として上下に設定を調整することができます。 値が小さい方がより高感度になり、値が大きい方が、より低感度になります。

#### ButtonStateCallback コールバック関数

``````````.cpp
void (*)(std::vector<ButtonState>, ButtonInfo, void*);
``````````

タッチボタンへのタッチが検出された際に呼び出されるユーザー定義コールバック関数です。第一引数に `ButtonState` 型の変数によりボタンのタッチ状態が返されます。第二引数は `ButtonInfo` 型の変数により、タッチ検出の内部状態が返されます。これは通常は利用しません。第三引数には、ユーザー指定の任意のデータが返されます。

コールバック関数は、指の接触が検知されている状態（`ButtonState::pushed_`）のとき、及び、接触検知状態から指が離されたとき（`ButtonState::released_`）の場合のみ呼ばれます。言い換えると、ステートが `ButtonState::none_` のときにはコールバック関数は呼ばれないことに留意してください。

### 環境センサー制御

#### 環境センサーについて

環境センサー対応は libtumbler のオプション対応であり、デフォルトでは無効です。`--enable-envsensor` オプションをつけて configure した場合のみ有効化されます。また、環境センサー自体も Tumbler の OS の初期設定で無効化されています。上記記載の通り、`raspi-config` コマンドを用いることで、OS の I2C サポートを有効にする必要があります。OS の I2C サポートが無効の場合、サンプルプログラムを実行した場合等に、I2C 経由の通信を行うことができず、`Unable to open I2C device: No such file or directory` 等の実行時エラーが発生することに留意してください。

環境センサーは BME280 を搭載しています。Tumbler の CPU 利用状況の影響を受け、温度は外気温より高めに測定される場合があります。このため、Tumbler の利用状況に応じてキャリブレーションをする必要性があることに留意してください。詳細は、下記をご参照ください。

#### EnvSensor クラス

EnvSensor クラスは、Tumbler に内蔵された環境センサーを表すクラスです。シングルトン・インスタンスとして、複数のスレッドから利用することができます。このクラスを用いることで、気温、湿度、気圧を取得することができます。

サンプルプログラムは [examples/envsensor.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/envsensor.cpp) を参照してください。

##### インスタンスの取得

``````````.cpp
EnvSensor& sensor = EnvSensor::getInstance();
``````````

以上のように EnvSensor クラスのインスタンスを取得することができます。

##### temperature()

``````````.cpp
float EnvSensor::temperature()
``````````

温度の測定結果（摂氏度）を返します。温度の測定結果は、Tumbler の CPU がアイドルの場合に外気温となるように調整されています。この結果についてはキャリブレーションを行う必要がある場合があります。

##### humidity()

``````````.cpp
float EnvSensor::humidity()
``````````

相対湿度の測定結果（％）を返します。相対湿度の測定結果は、温度の測定結果の影響を受けます。

##### pressure()

``````````.cpp
float EnvSensor::pressure()
``````````

気圧の測定結果（hPa）を返します。

##### calibrateTemperature()

``````````.cpp
float EnvSensor::calibrateTemperature(float temp)
``````````

内蔵された温度センサーは、Tumbler 本体から発生する熱の影響を受け、測定値が上振れする場合があります。温度センサーの計測値が上振れした場合、湿度（相対湿度）センサーの計測値が下振れすることになります。熱は特に CPU から発生し、CPU が熱飽和している状況が継続した場合、継続時間に応じて、最大で５度程度、高い値が計測される傾向があります。デフォルトでは、CPU がアイドル状態の場合に外気温となるよう調整されていますが、ユーザーアプリケーションに応じた CPU 使用率の平均状況が分かっている場合には、第一引数の `temp` を用いて、キャリブレーション値（一般的には正の値）を指定することが有効です。`temp` に指定された固定値は、温度センサーの実測値から減算され、温度センサー計測値として `temperature()` 関数の戻り値として返されます。これは、湿度センサーの相対湿度計算にも利用されます。

温度センサーのキャリブレーションは、別途温度計を用意して行うことができます。ユーザーアプリケーションが起動している状態で５分程度の適当な時間をおき、Tumbler 本体の熱状況を安定させます。このときの `temperature()` 関数の出力値と、別途用意した温度計の計測値との差分が `temp` に指定すべきキャリブレーション値となります。

### 光センサー制御

#### 光センサーについて

光センサーは標準装備されていますが、 libtumbler ではオプション対応であり、デフォルトでは無効です。`--enable-lightsensor` オプションをつけて configure した場合のみ有効化されます。また、光センサー自体も Tumbler の OS の初期設定で無効化されています。上記記載の通り、`raspi-config` コマンドを用いることで、OS の I2C サポートを有効にする必要があります。OS の I2C サポートが無効の場合、サンプルプログラムを実行した場合等に、I2C 経由の通信を行うことができず、`Unable to open I2C device: No such file or directory` 等の実行時エラーが発生することに留意してください。

光センサーは LTR-329ALS を搭載しています。感度帯域等については、センサーのデータシートを御覧ください。

#### LightSensor クラス

LightSensor クラスは、Tumbler に内蔵された光センサーを表すクラスです。シングルトン・インスタンスとして、複数のスレッドから利用することができます。このクラスを用いることで、照度[lux]を取得することができます。

サンプルプログラムは [examples/lightsensor.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/lightsensor.cpp) を参照してください。

##### インスタンスの取得

``````````.cpp
LightSensor& sensor = LightSensor::getInstance();
``````````

以上のように LightSensor クラスのインスタンスを取得することができます。

##### light()

``````````.cpp
unsigned int LightSensor::light()
``````````

唯一のメンバー関数であり、照度[lux]を返します。照度センサーは、Tumbler 上部パネルの正面側にあるマイク穴の下に設置されています。このため、上部パネルに手をかざすようにすると、相対的に小さい照度測定結果を得ることができます。

### 赤外線 I/O 制御

#### 赤外線 I/O について

赤外線 I/O は、以下の 2 つの機能として利用することができます。この 2 つの機能は、同時に利用することも可能です。同時に利用するサンプルプログラムは [examples/irall.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/irall.cpp) を参照してください。

##### 正面近接センサー機能

Tumbler から放射される赤外線の反射を、Tumbler 正面側に設置された赤外線受光部で検出することで、Tumbler 正面に物体が存在していることを検出します。センサー感度を設定することができ、最大感度の場合で約 1 m、最小感度の場合で約 30 cm が検出範囲となります。この機能は、Tumbler の正面に人が接近したことを検知するために利用することができます。

サンプルプログラムは [examples/irproximitysensor.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/irproximitysensor.cpp) を参照してください。

##### 外部赤外線信号受信機能

Tumbler 正面側に設置された赤外線受光部で、一般の赤外線リモコン等の赤外線信号を受信することができる機能です。一般の赤外線リモコン等で、Tumbler を遠隔操作することが可能となります。

サンプルプログラムは [examples/irsignalreceiver.cpp](https://github.com/FairyDevicesRD/tumbler/blob/master/libtumbler/examples/irsignalreceiver.cpp) を参照してください。

#### IRProximitySensor クラス

IRProximitySensor クラスは、赤外線 I/O を正面近接センサーとして利用するためのクラスです。シングルトン・インスタンスとして、複数のスレッドから利用することができます。

##### インスタンスの取得

``````````.cpp
IRProximitySensor& sensor = IRProximitySensor::getInstance(IRProximityDetectionCallback func, void* userdata);
``````````

第一引数は、正面近接センサーが物体を検知した際に呼ばれるコールバック関数を指定します。第二引数では、コールバック関数に渡されるユーザー定義の任意データを指定することができます。

##### IRProximitySensor::Sensitivity 列挙型

``````````.cpp
	enum class Sensitivity
	{
		high_,   //!< 高感度（概ね 1 m 以上程度で反応）
		medium_, //!< 中感度（概ね 60cm 程度で反応）
		low_,    //!< 低感度（概ね 30cm 程度で反応）
	};
``````````

正面近接センサーの感度を指定します。赤外線信号の反射で検出するという原理上、検知距離は、物体の赤外線の反射しやすさや、反射面の大きさに依存して若干変動することに留意してください。

##### start()

``````````.cpp
int IRProximitySensor::start()
int IRProximitySensor::start(IRProximitySensor::Sensitivity p)
``````````

正面近接センサーを開始します。デフォルトでは最大感度で開始されます。感度を弱めたい場合は、第一引数で感度指定可能なオーバーロード関数を利用することができます。感度は、高・中・低の三段階です。この関数が呼ばれると、Tumbler 上部の LED リングから、近接検知用赤外線信号の発振が開始されます。赤外線を肉眼で見ることはできませんが、スマートフォンのカメラ等をかざすことで、カメラ画面を通して薄くピンク色に光っている様子を観察することができます。

##### stop()

``````````.cpp
int IRProximitySensor::stop()
``````````

正面近接センサーを終了します。この関数を呼ぶことで、近接検知用赤外線信号の発振も停止します。

#### IRProximityDetectionCallback コールバック関数

``````````.cpp
void (*)(uint32_t tick, void* userdata);
``````````

第一引数には、近接検知されたマイクロ秒が返されます。この値は、約 1.2 時間で周回しますが、tick 値の差分は常に期待される値を返すため、周回することについて利用側で留意する必要はありません。第二引数には、ユーザー定義の任意のデータが渡されます。

#### IRSignalReceiver クラス

IRSignalReceiver クラスは、赤外線 I/O を外部からの赤外線信号の受信機として利用するためのクラスです。シングルトン・インスタンスとして、複数のスレッドから利用することができます。

##### インスタンスの取得

``````````.cpp
IRSignalReceiver& receiver = IRSignalReceiver::getInstance(IRSignalReceiptCallback func, void* userdata);
``````````

第一引数では、外部からの赤外線信号を受信した際に呼ばれるコールバック関数を指定します。第二引数では、コールバック関数に渡されるユーザー定義の任意データを指定することができます。

##### start()

``````````.cpp
int IRSignalReceiver::start()
``````````

外部からの赤外線信号受信を開始します。赤外線受光部は Tumbler 正面に搭載されていますので、赤外線リモコンは、概ね正面方向から操作する必要があります。

##### stop()

``````````.cpp
int IRSignalReceiver::stop()
``````````

外部からの赤外線信号受信を終了します。

#### IRSignalReceiptCallback コールバック関数

``````````.cpp
void (*)(uint32_t signalHash, uint32_t tick, void* userdata);
``````````

第一引数には、受信した赤外線信号に基づき計算されたハッシュ値が返されます。このハッシュ値は、ある信号に対して一意に定まる値であり、コールバック関数内でユーザープログラムが赤外線信号を識別するために利用することができます。第二引数には、近接検知されたマイクロ秒が返されます。この値は、約 1.2 時間で周回しますが、tick 値の差分は常に期待される値を返すため、周回することについて利用側で留意する必要はありません。第三引数には、ユーザー定義の任意のデータが渡されます。

赤外線リモコンは、普通にボタンを押すと、同一の信号が複数回送信されるものがあります。その場合、このコールバック関数も同一ハッシュ値で複数回呼ばれます。このようなときに、長押しと判定しても良いですが、１度しか対応する処理を実行したくない場合には、同一ハッシュ値だった場合に tick 値を確認し、一定時間以下だった場合は、単一命令とみなすという実装をユーザープログラム側で加えることができます。

## libtumbler ライセンス情報

本ライブラリは Apache-2.0 ライセンスに基づき提供されています。本ライブラリは、以下のオープンソースライブラリを含みます。

- raspberry-pi-bme280（https://github.com/andreiva/raspberry-pi-bme280） src/thirdparty/raspberry-pi-bme280/



