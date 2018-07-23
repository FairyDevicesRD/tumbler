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
#include <EEPROM.h>

// ローカル共通処理.
#include "EEPROMTable.h"
#include "SerialIO.h"
#include "Constants.h"
#include "CapacitiveSensor.h"

// Module系は他のヘッダよりも後に参照すること.
#include "Module.h"
#include "TouchSensor.h"
#include "BMEControl.h"
#include "LEDRing.h"
#include "EEPROMControl.h"

// 制御対象
sonar::BMEControl bme280_ = sonar::BMEControl();
sonar::LEDRing   ledring_ = sonar::LEDRing();
sonar::EEPROMControl eep_ = sonar::EEPROMControl();
sonar::TouchSensor touch_ = sonar::TouchSensor();

// # 起動時に確保するメモリ領域

// ## アニメーション関係
uint32_t frames_ = 0;     //!< 累積フレーム数
uint32_t start_time_ = 0; //!< アップデータ開始時刻
uint32_t end_time_ = 0;   //!< アップデータ終了時刻

// Version Info.
const uint8_t mejorVersion = 1;
const uint8_t minorVersion = 1;


void touchLEDShow();

void setup()
{
  // 通信系の開始（グローバル変数として利用されるが、利用の仕方には注意）
  //Serial.begin(57600);  
  Serial.begin(19200);  
  // 38400 bps, 4800 byte/sec, 48 byte/10msec(=100fps)
  // 19200 bps, 2400 byte/sec  60 byte /25msec(=40fps)
  while(!Serial){
  }
  Wire.begin();
  delay(500);

  // モジュールの初期化(順番大事かも).
  eep_.init(mejorVersion, minorVersion);
  touch_.init();
  bme280_.init();
  ledring_.init();

}

void loop()
{
  int8_t ret = 0;
  start_time_ = millis();

  // シリアル通信経由の命令受信の確認
  if(serialRecv()){

    // Memo:一度にいずれか1つのみ処理される.
    ret = bme280_.recv(getSerialComType(), getSerialComSub(), getSerialComBody(), getSerialComLen());
    ret = ledring_.recv(getSerialComType(), getSerialComSub(), getSerialComBody(), getSerialComLen());
    ret = eep_.recv(getSerialComType(), getSerialComSub(), getSerialComBody(), getSerialComLen());
    ret = touch_.recv(getSerialComType(), getSerialComSub(), getSerialComBody(), getSerialComLen());
    Serial.flush();
  }
  if(ret != 0){
    // リセット処理.
    touch_.reset();
  }

  // タッチセンサーの動作確認.
  touchLEDShow();

  // モジュールの更新と実行.
  int8_t busyFlg = 0; // 重い処理は同時に行わない.
  ledring_.update(frames_);
  if(!busyFlg) busyFlg = touch_.update(frames_);
  if(!busyFlg) busyFlg = bme280_.update(frames_);
  end_time_ = millis();
  ++frames_;
  // タイムキープしない.


}



// タッチセンサーの動作にあわせて点灯.
void touchLEDShow()
{
  if(ledring_.getMode() == 9){
    if(touch_.getCSState(0) == TOUCH_STATE_SHORT)
      ledring_.setFrame(16,64,0,0);
    else if(touch_.getCSState(0) == TOUCH_STATE_LONG)
      ledring_.setFrame(16,255,0,0);
    else
      ledring_.setFrame(16,0,0,0);

    if(touch_.getCSState(1) == TOUCH_STATE_SHORT)
      ledring_.setFrame(0,0,64,0);
    else if(touch_.getCSState(1) == TOUCH_STATE_LONG)
      ledring_.setFrame(0,0,255,0);
    else
      ledring_.setFrame(0,0,0,0);

    if(touch_.getCSState(2) == TOUCH_STATE_SHORT)
      ledring_.setFrame(2,0,0,64);
    else if(touch_.getCSState(2) == TOUCH_STATE_LONG)
      ledring_.setFrame(2,0,0,255);
    else
      ledring_.setFrame(2,0,0,0);

    if(touch_.getCSState(3) == TOUCH_STATE_SHORT)
      ledring_.setFrame(4,64,0,0);
    else if(touch_.getCSState(3) == TOUCH_STATE_LONG)
      ledring_.setFrame(4,255,0,0);
    else
      ledring_.setFrame(4,0,0,0);
  }
}
