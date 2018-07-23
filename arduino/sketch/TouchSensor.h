#ifndef __CS_SUB__
#define __CS_SUB__

#define T_SENSOR_NUM           4

// タッチセンサーの状態フラグ.
#define TOUCH_STATE_OFF        0
#define TOUCH_STATE_SHORT      2
#define TOUCH_STATE_LONG       4

// タッチセンサーの変化フラグ.
#define TOUCH_CHANGE           1
#define TOUCH_CHANGE_OFFOFF 0x00
#define TOUCH_CHANGE_OFFS   0x01 
#define TOUCH_CHANGE_SS     0x02
#define TOUCH_CHANGE_SL     0x03
#define TOUCH_CHANGE_LL     0x04
#define TOUCH_CHANGE_SOFF   0x05
#define TOUCH_CHANGE_LOFF   0x07

#define touchInterval        200 // タッチ読み取り間隔.
#define touchLongTime          3 // 長押し判定時間.
#define WAIT_SLEEP          (-2) // 離した後の待ち時間(連打禁止).

namespace sonar
{
  class TouchSensor : public Module
  {
  public:
    TouchSensor() :
      Module("TouchSensor")
    {}

    int8_t init() override
    {
      Serial.print("TouchSensor initializing ... ");
      
      int i;
      for (i = 0; i < T_SENSOR_NUM; i++) {
        capSensor[i] = CapacitiveSensor( i + 3 + i, i + 4 + i);
        cs_val[i] = 0;
        touchState[i] = TOUCH_STATE_OFF;
        touchChange[i] = TOUCH_CHANGE_OFFOFF;
      }
      if(EEPROM.read(EEP_POS_INIT_FLG) == 1){
        cs_threshVal[0] = EEPROM.read(EEP_POS_CS_THRESH_1);
        cs_threshVal[1] = EEPROM.read(EEP_POS_CS_THRESH_2);
        cs_threshVal[2] = EEPROM.read(EEP_POS_CS_THRESH_3);
        cs_threshVal[3] = EEPROM.read(EEP_POS_CS_THRESH_4);
      }
      tcss_timer_start = millis();
      reportFlg = 1;
      targetKey = 0;

      Serial.println(" [ OK ]");
      return 0;
    }
    
    // 更新処理.
    int8_t update(uint32_t frames) override
    {
      int8_t ret = 0;
      char updated = 0;
      tcss_timer_interval = millis() - tcss_timer_start;

      if (targetKey < 4){
        if(waitSendFlag[targetKey] < 0){
          waitSendFlag[targetKey]++;
        } else {
          // cs_val[targetKey] = capSensor[targetKey].capacitiveSensor(80);
          cs_val[targetKey] = capSensor[targetKey].capacitiveSensor(40);
          ret = 1;
        }
        targetKey++;
      }
      if(targetKey == 4){
        updated = updateCSState();
        if(reportFlg && updated) reportCSState();
        targetKey++;
      }

      if (tcss_timer_interval > touchInterval && targetKey >= 4) {
        tcss_timer_start = millis();
        targetKey = 0;
      }
      return ret;
    }
    
    // シリアル受信処理.
    int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length) override
    {
      if(strcmp(type, "TSNS") == 0){
        if(subtype == 0){
          // ボタン状態取得.
          reportCSState();
        }
        else if(subtype == 1){
          // 閾値取得.
          reportCSThreshVal();
        }
        else if(subtype == 2 || subtype == 3){
          // 閾値設定(保存しない).
          if(length >= 4){
            cs_threshVal[0] = body[0];
            cs_threshVal[1] = body[1];
            cs_threshVal[2] = body[2];
            cs_threshVal[3] = body[3];
            Serial.print("set thresh");
            if(subtype == 3){
              // 閾値設定(保存する).
              EEPROM.write(EEP_POS_CS_THRESH_1, body[0]);
              EEPROM.write(EEP_POS_CS_THRESH_2, body[1]);
              EEPROM.write(EEP_POS_CS_THRESH_3, body[2]);
              EEPROM.write(EEP_POS_CS_THRESH_4, body[3]);
              Serial.print(" & eeprom update.");
            }
            Serial.println("");
          }
        }
        else if(subtype == 4){
          if(length > 0){
            reportFlg = body[0];
          }
          Serial.print("Report:");
          if(reportFlg) Serial.println("ON");
          else Serial.println("OFF");

        }
        else if(subtype == 9){
          // 計測値取得.
          reportCSVal();
        }
        else{
          Serial.println("no command");
        }
      }
      return 0;
    }

    // 状態の更新処理.
    uint8_t updateCSState()
    {
      uint8_t change = 0;
      int i;
      for(i = 0;i < T_SENSOR_NUM; ++i){
        if(cs_val[i] > cs_threshVal[i]){
          if(touchState[i] == TOUCH_STATE_OFF){
            repeatCount[i] = 0;
            touchState[i] = TOUCH_STATE_SHORT;
            touchChange[i] = TOUCH_CHANGE_OFFS;
            change = 1;
          }
          else if(touchState[i] == TOUCH_STATE_SHORT){
            repeatCount[i]++;
            if(repeatCount[i] > touchLongTime){
              touchState[i] = TOUCH_STATE_LONG;
              touchChange[i] = TOUCH_CHANGE_SL;
              change = 1;
            } else{
              touchChange[i] = TOUCH_CHANGE_SS;
            }
          }
          else if(touchState[i] == TOUCH_STATE_LONG){
              touchChange[i] = TOUCH_CHANGE_LL;
          }
        }
        else{
          // [cs_val < thresh].
          if(touchState[i] == TOUCH_STATE_SHORT){
            touchChange[i] = TOUCH_CHANGE_SOFF;
            waitSendFlag[i] = WAIT_SLEEP;
            change = 1;
          }
          else if(touchState[i] == TOUCH_STATE_LONG){
            touchChange[i] = TOUCH_CHANGE_LOFF;
            waitSendFlag[i] = WAIT_SLEEP;
            change = 1;
          } else {
            touchChange[i] = TOUCH_CHANGE_OFFOFF;
          }
          touchState[i] = TOUCH_STATE_OFF;
        }
      }

      return change;
    } 

    // pi側に状態変化を報告.
    void reportCSState()
    {
      Serial.print("TS");
      for(int i = 0; i < 4; ++i){
        Serial.print(touchChange[i]);
        Serial.print(" ");
      }
      Serial.println("");
      Serial.flush();
    }

    // センサー閾値を報告.
    void reportCSThreshVal()
    {
      Serial.print("TT");
      for(uint8_t i = 0; i < 4; ++i){
        Serial.print(cs_threshVal[i]);
        Serial.print(" ");
      }
      Serial.println("");
      Serial.flush();
    }

    // センサー値を報告.
    void reportCSVal()
    {
      Serial.print("TV");
      for(uint8_t i = 0; i < 4; ++i){
        if(cs_val[i] < 255)
        {
          Serial.print(cs_val[i]);
          Serial.print(" ");
        }
        else
        {
          Serial.print("255 ");
        }
      }
      Serial.println("");
      Serial.flush();
    }
    
    // 接触センサーの値.
    long getCSVal(int num)
    {
      if(num < T_SENSOR_NUM)
        return cs_val[num];
      else 
        return 0;
    }

   // センサーの状態を取得.
    uint8_t getCSState(int num)
    {
      if(num < T_SENSOR_NUM)
        return touchState[num];
      else 
        return 0;
    }

    // 接触センサーの閾値.
    uint8_t getCSThreshVal(int num)
    {
      if(num < T_SENSOR_NUM)
        return cs_threshVal[num];
      else 
        return 0;
    }

    void setCSThreshVal(int num, uint8_t val)
    {
      if(num < T_SENSOR_NUM)
        cs_threshVal[num] = val;
    }

    // 設定値のリセット.
    void reset()
    {
      cs_threshVal[0] = EEPROM.read(EEP_POS_CS_THRESH_1);
      cs_threshVal[1] = EEPROM.read(EEP_POS_CS_THRESH_2);
      cs_threshVal[2] = EEPROM.read(EEP_POS_CS_THRESH_3);
      cs_threshVal[3] = EEPROM.read(EEP_POS_CS_THRESH_4);
    }
  private:
    CapacitiveSensor capSensor[T_SENSOR_NUM];
    unsigned long tcss_timer_start = 0;
    unsigned long tcss_timer_interval = 0;

    long cs_val[T_SENSOR_NUM];
    uint8_t cs_threshVal[T_SENSOR_NUM]={50,50,50,50};

    uint8_t waitSendFlag[T_SENSOR_NUM] = {0,0,0,0}; // 送信準備OKフラグ.
    uint8_t repeatCount[T_SENSOR_NUM]  = {0,0,0,0}; // 長押しカウント.
    uint8_t touchState[T_SENSOR_NUM]   = {0,0,0,0}; // 状態.
    uint8_t touchChange[T_SENSOR_NUM]  = {0,0,0,0}; // 状態変化.
    uint8_t reportFlg = 0;
    uint8_t targetKey = 0;
  };  
}


#endif
