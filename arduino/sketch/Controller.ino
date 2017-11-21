/**
* @file Controller.ino
 * @brief エントリポイント. 各モジュールの実装は、各モジュールクラスで行われ、全体制御のみがこのファイルに実装される.
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/06/14 
 */

//#define NDEBUG

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <SSCI_BME280.h>

#include "Module.h"
#include "Constants.h"
#include "BME280.h"
#include "LEDRing.h"

// 制御対象
//sonar::BME280 bme280_ = sonar::BME280();
sonar::LEDRing ring_ = sonar::LEDRing(false);

// # 起動時に確保するメモリ領域

// ## アニメーション関係
uint32_t frames_ = 0;     //!< 累積フレーム数
uint32_t start_time_ = 0; //!< アップデータ開始時刻
uint32_t end_time_ = 0;   //!< アップデータ終了時刻

// ## シリアル通信関係
//
// ### シリアル通信仕様定義
// type        | 4byte | コマンドタイプ（例: LED; LED に対する制御命令という意味）
// subtype     | 1byte | コマンドサブタイプ（例: 0x01; LED に対する制御命令のうち 1 番であるという意味）
// data_length | 1byte | データ長（ヘッダを含まず）; ここまでヘッダとして取り扱う（合計 6 byte）
// data        | -     | データ本体. データ長 byte 

const uint8_t serial_header_length_ = 6;  //!< ヘッダ長
char serial_com_type_[5];        //!< 4 byte + \0
uint8_t serial_com_subtype_ = 0;     //!< 1 byte
uint8_t serial_com_body_length_ = 0; //!< 1 byte（データ長）
char serial_com_body_[64];      //!< データ長 byte, データ本体
uint8_t serial_cur_ = 0;         //!< シリアル通信データ本体受信オフセット
bool serial_eob_ = false;        //!< ボディ読み終わり
uint32_t last_serial_recv_;      //!< 最終受信時刻

// 実行時メモリを確認したいときのみ有効にすること. 他の部分に不具合が出る.
#ifdef RUNTIME_MEMORY_CHECK
int printFreeRAM () {
	extern int __heap_start, *__brkval;
	int v;
	int ram = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
	Serial.print("Free RAM: ");
	Serial.print(ram);
	Serial.println(" bytes");
}
#endif

void setup()
{
	// 通信系の開始（グローバル変数として利用されるが、利用の仕方には注意）
	Serial.begin(19200);	
	// 38400 bps, 4800 byte/sec, 48 byte/10msec(=100fps)
	// 19200 bps, 2400 byte/sec  60 byte /25msec(=40fps)
	Wire.begin();
	delay(500);
	
	// モジュールの初期化   
	//Serial.println("sonar.ai sensor subsystem w/ arduino uno, starting-up");
	//Serial.println("-----------------------------------------------------");
	//bme280_.init();
	ring_.init();
#ifdef RUNTIME_MEMORY_CHECK
	printFreeRAM();
#endif
   
}

//#define SERIALDEBUG_HEADER_
//#define SERIALDEBUG_BODY_
char tmp_[64];
char ch;
int serialRecv()
{
	uint8_t bytes = Serial.available(); // 受信バッファにデータが来ているか？受信バッファサイズは公式ドキュメントが誤りで実際は 32 byte なので注意.
	//last_serial_recv_ = millis();
	if(bytes == 0){
		return 0;
	}
	// 受信バッファにデータが来ているときはその長さ分だけ読み出す
	Serial.readBytes(tmp_, bytes);
	
	// 受信完了信号を返す
	//if(serial_com_body_length_ != 0 &&　serial_cur_+bytes == serial_com_body_length_ + serial_header_length_){
	//	Serial.print('0');
	//	Serial.flush();
	//}

	// 受信内容の解析
	for(uint8_t i=0;i<bytes;++i){
		ch = tmp_[i];
		if(serial_cur_ < 4){
			serial_com_type_[serial_cur_] = ch; // コマンド種別は 4 文字
		}else if(serial_cur_ == 4){
			serial_com_type_[4] = '\0'; // null 文字で閉じておく
			serial_com_subtype_ = static_cast<uint8_t>(ch); // 5 文字目はサブタイプ
		}else if(serial_cur_ == 5){ // 6 文字目はデータ長
			serial_com_body_length_ = static_cast<uint8_t>(ch);
			// ヘッダ受信完了
#ifdef SERIALDEBUG_HEADER_
			uint32_t t = millis();
			Serial.print("Header Received: time=");
			Serial.print(t);
			Serial.print(", type=");
			Serial.print(serial_com_type_);
			Serial.print(", subtype=");
			Serial.print(serial_com_subtype_);
			Serial.print(", length=");	
			Serial.println(serial_com_body_length_);		   
#endif			
			if(serial_com_body_length_ == 0){ // データ長がゼロの場合は、ここで終了とする
				serial_eob_ = true;
				break;
			}
		}else{
			serial_com_body_[serial_cur_-serial_header_length_] = ch;
			if(serial_cur_ == serial_com_body_length_ + serial_header_length_ - 1){	
#ifdef SERIALNDEBUG_BODY_
				uint32_t t = millis();
				Serial.print("Body Received: time=");
				Serial.print(t);
				Serial.print(", body=");
				for(uint8_t i=0;i<serial_com_body_length_;++i){
					Serial.print(serial_com_body_[i], HEX);
					Serial.print(" ");					
				}
				t = millis();
				Serial.print(", time=");
				Serial.println(t);
#endif
				serial_eob_ = true;
				break;
			}
		}
		serial_cur_++;
	}

	if(serial_eob_){
		// 受信完了信号を返す
		Serial.print('1');
		Serial.flush();
	}
	return serial_cur_;
}

void loop()
{
	start_time_ = millis();	
	// シリアル通信の確認
	serialRecv();	
	// シリアル通信経由の命令受信の確認
	if(serial_eob_){
		// bme280_.recv(serial_com_type_, serial_com_subtype_, serial_com_body_, serial_com_body_length_);
		ring_.recv(serial_com_type_, serial_com_subtype_, serial_com_body_, serial_com_body_length_);
		serial_eob_ = false;
		serial_cur_ = 0;
		serial_com_body_length_ = 0;
	}
	// モジュールの更新と実行
	// bme280_.update(frames_);
	ring_.update(frames_);
	// シリアル通信のコンシステンシー維持のためのタイムアウト	
	/*
	if(last_serial_recv_ < start_time_ && 100 < start_time_ - last_serial_recv_){
		serial_eob_ = false;
		serial_cur_ = 0;
		serial_com_body_length_ = 0;
	}
	*/
	end_time_ = millis();
	++frames_;
	// タイムキープしない
}


