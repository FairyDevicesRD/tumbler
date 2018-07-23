#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EEPROM.h>
#include "EEPROMTable.h"
#include "CapacitiveSensor.h"
#include "SerialIO.h"

uint32_t serial_start_time_ = 0;     //!< アップデータ開始時刻.
uint32_t serial_last_recv_ = 0;      //!< 最終受信時刻.

#define SERIAL_HEADER_LEN 6          //!< ヘッダ長.
char serial_com_type_[5];            //!< 4 byte + \0
uint8_t serial_com_subtype_ = 0;     //!< 1 byte
uint8_t serial_com_body_length_ = 0; //!< 1 byte（データ長）
uint8_t serial_cur_= 0;              //!< 1 byte
char serial_com_body_[64];           //!< データ長 byte, データ本体.
bool serial_eob_ = false;            //!< ボディ読み終わり.

// シリアル受信処理.
// 1回の処理で最大1コマンド分受信する.
int serialRecv()
{
  static char tmp_[64];
  char ch;

  serial_start_time_ = millis();

  // 割り込み処理は発生しないと仮定して、前回読み込んだ内容は処理済なのでクリアする。
  if(serial_eob_){
    serial_eob_ = false;
    serial_cur_ = 0;
    serial_com_body_length_ = 0;
  }

  // タイムアウト処理
  if(serial_cur_ > 0){
    if(serial_last_recv_ < serial_start_time_){
      if(2000 < serial_start_time_ - serial_last_recv_){
        serial_eob_ = false;
        serial_cur_ = 0;
        serial_com_body_length_ = 0;
        Serial.print("Serial recv time out");
        Serial.println(serial_com_type_);
        serial_last_recv_ = serial_start_time_;
      }
    } else if(serial_start_time_ < 2000 && serial_last_recv_ > 2000) {
      // ループ対策。
      serial_last_recv_ = serial_start_time_;
    }
  }


  // 受信バッファにデータが来ているか？
  // 受信バッファサイズは公式ドキュメントが誤りで実際は 32 byte なので注意.
  uint8_t bytes = Serial.available();
  uint8_t leftBytes = bytes;
  if(bytes == 0){
    return 0;
  }
  serial_last_recv_ = serial_start_time_;

/* recieve check
  Serial.print(bytes + serial_cur_);
  Serial.println("bytes");
  Serial.println(serial_last_recv_);
*/

  // ヘッダ部分の読み込み.
  uint8_t leftHeaderSize = (serial_cur_ < SERIAL_HEADER_LEN) ? SERIAL_HEADER_LEN - serial_cur_ : 0;
  if(leftHeaderSize > 0)
  {
    // 残りデータ長に合わせて取得するデータを調整する.
    if(leftBytes <= leftHeaderSize)
    {
      Serial.readBytes(tmp_, leftBytes);
      bytes = leftBytes;
    }
    else
    {
      Serial.readBytes(tmp_, leftHeaderSize);
      bytes = leftHeaderSize;
    }
    leftBytes -= bytes;

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
        if(serial_com_body_length_ == 0){ // データ長がゼロの場合は、ここで終了とする
          serial_eob_ = true;
          break;
        }
      }
      serial_cur_++;
    }
  }

  // データ部分の読み込み.
  if(serial_eob_ == false && leftBytes > 0)
  {
    // 残りデータ長に合わせて取得するデータを調整する.
    uint8_t leftBodySize = serial_com_body_length_ - (serial_cur_ - SERIAL_HEADER_LEN);
    if(leftBytes <= leftBodySize)
    {
      Serial.readBytes(tmp_, leftBytes);
      bytes = leftBytes;
    }
    else
    {
      Serial.readBytes(tmp_, leftBodySize);
      bytes = leftBodySize;
    }
    leftBytes -= bytes;

    for(uint8_t i=0;i<bytes;++i){
      ch = tmp_[i];
      serial_com_body_[serial_cur_-SERIAL_HEADER_LEN] = ch;
      serial_cur_++;
    }
    if(serial_cur_ == serial_com_body_length_ + SERIAL_HEADER_LEN){ 
      serial_eob_ = true;
    }
  }


  return serial_eob_;
}

char *getSerialComType()
{
  return serial_com_type_;
}

char getSerialComSub()
{
  return serial_com_subtype_;
}

char *getSerialComBody()
{
  return serial_com_body_;
}

char getSerialComLen()
{
  return serial_com_body_length_;
}

bool getSerialComEof()
{
  return serial_eob_;
}

