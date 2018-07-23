namespace sonar
{
  class EEPROMControl : public Module
  {
  public:
    EEPROMControl() :
      Module("EEPROMControl")
    {}

    int8_t init() override
    {
      Serial.print("eepromControl initializing ... ");
      Serial.println(" [ OK ]");
      return 0;
    }

    int8_t init(uint8_t mejorVer_, uint8_t minorVer_)
    {
      Serial.print("eepromControl initializing ... ");
      initVersion(mejorVer_, minorVer_);
      Serial.println(" [ OK ]");
      return 0;
    }

    int8_t update(uint32_t frames) override
    {
      return 0;
    }

    int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length) override
    {
      int8_t ret = 0;
      if(strcmp(type, "EEPW") == 0){
        // EEPROM書き込み要求.
        switch(subtype){
        case 0:
          // バージョン更新.
          if(length > 0){
            EEPROM.write(EEP_POS_VERSION_MJR, body[0]);
          } else{
            EEPROM.write(EEP_POS_VERSION_MJR, 1);
          }
          if(length > 1){
            EEPROM.write(EEP_POS_VERSION_MNR, body[1]);
          } else{
            EEPROM.write(EEP_POS_VERSION_MNR, 0);
          }
          Serial.println("eeprom Ver. set ");
          break;
        case 1:
          // タッチセンサー初期化.
          EEPROM.write(EEP_POS_INIT_FLG, 1);
          if(length < 4){
            EEPROM.write(EEP_POS_CS_THRESH_1, 50);
            EEPROM.write(EEP_POS_CS_THRESH_2, 50);
            EEPROM.write(EEP_POS_CS_THRESH_3, 50);
            EEPROM.write(EEP_POS_CS_THRESH_4, 50);
          } else {
            EEPROM.write(EEP_POS_CS_THRESH_1, body[0]);
            EEPROM.write(EEP_POS_CS_THRESH_2, body[1]);
            EEPROM.write(EEP_POS_CS_THRESH_3, body[2]);
            EEPROM.write(EEP_POS_CS_THRESH_4, body[3]);
          }
          Serial.println("eeprom touch setting ");
          ret = 9;
          break;
        case 2:
          // BMEセンサー補正値の設定.( 0x80 = 0 )
          if(length < 3){
            EEPROM.write(EEP_POS_BME_TMP, 0x80);
            EEPROM.write(EEP_POS_BME_HUM, 0x80);
            EEPROM.write(EEP_POS_BME_PRS, 0x80);
            EEPROM.write(EEP_POS_BME_COEF_TMP, 100);
            EEPROM.write(EEP_POS_BME_COEF_HUM, 100);
            EEPROM.write(EEP_POS_BME_COEF_PRS, 100);
          } else if(length < 6){
            EEPROM.write(EEP_POS_BME_TMP, body[0]+0x80);
            EEPROM.write(EEP_POS_BME_HUM, body[1]+0x80);
            EEPROM.write(EEP_POS_BME_PRS, body[2]+0x80);
          } else {
            EEPROM.write(EEP_POS_BME_TMP, body[1]+0x80);
            EEPROM.write(EEP_POS_BME_HUM, body[3]+0x80);
            EEPROM.write(EEP_POS_BME_PRS, body[5]+0x80);
            EEPROM.write(EEP_POS_BME_COEF_TMP, body[0]);
            EEPROM.write(EEP_POS_BME_COEF_HUM, body[2]);
            EEPROM.write(EEP_POS_BME_COEF_PRS, body[4]);
          }
          Serial.println("eeprom bme setting ");
          ret = 9;
          break;
        case 99:
          // EEPROM初期化(出荷状態).
          EEPROM.write(EEP_POS_VERSION_MJR, 1);
          EEPROM.write(EEP_POS_VERSION_MNR, 0);
          EEPROM.write(EEP_POS_INIT_FLG, 1);
          EEPROM.write(EEP_POS_CS_THRESH_1, 50);
          EEPROM.write(EEP_POS_CS_THRESH_2, 50);
          EEPROM.write(EEP_POS_CS_THRESH_3, 50);
          EEPROM.write(EEP_POS_CS_THRESH_4, 50);
          EEPROM.write(EEP_POS_BME_TMP, 0x80);
          EEPROM.write(EEP_POS_BME_HUM, 0x80);
          EEPROM.write(EEP_POS_BME_PRS, 0x80);
          EEPROM.write(EEP_POS_BME_COEF_TMP, 100);
          EEPROM.write(EEP_POS_BME_COEF_HUM, 100);
          EEPROM.write(EEP_POS_BME_COEF_PRS, 100);
        
          Serial.println("eeprom initialized ");
          ret = 9;
          break;
        default:
          if(length < 2)  break;
          if(body[0] < 4)
          {
            EEPROM.write(EEP_POS_CS_THRESH + body[0] * 2, body[1]);
            Serial.println("eeprom Write");
            ret = 9;
          }

          break;
        }
      }
      if(strcmp(type, "EEPR") == 0){
        // EEPROM読み込み要求.
        switch(subtype){
        case 0:
          // バージョン情報
          Serial.print("Ver. ");
          Serial.print(EEPROM.read(EEP_POS_VERSION_MJR));
          Serial.print(".");
          Serial.print(EEPROM.read(EEP_POS_VERSION_MNR));
          Serial.println("");
          break;
        case 1:
          // タッチセンサー.
          {
            Serial.print("Touch Thresh:");
            Serial.print(EEPROM.read(EEP_POS_CS_THRESH_1));
            Serial.print(" ");
            Serial.print(EEPROM.read(EEP_POS_CS_THRESH_2));
            Serial.print(" ");
            Serial.print(EEPROM.read(EEP_POS_CS_THRESH_3));
            Serial.print(" ");
            Serial.print(EEPROM.read(EEP_POS_CS_THRESH_4));
            Serial.println("");
          }
          break;
        case 2:
          // BMEセンサー補正値.( 0x80 = 0 )
          {
            Serial.print("BME Adjust:");
            Serial.print(EEPROM.read(EEP_POS_BME_COEF_TMP));
            Serial.print(" ");
            Serial.print(EEPROM.read(EEP_POS_BME_TMP) - 0x80);
            Serial.print(" ");
            Serial.print(EEPROM.read(EEP_POS_BME_COEF_HUM));
            Serial.print(" ");
            Serial.print(EEPROM.read(EEP_POS_BME_HUM) - 0x80);
            Serial.print(" ");
            Serial.print(EEPROM.read(EEP_POS_BME_COEF_PRS));
            Serial.print("");
            Serial.print(EEPROM.read(EEP_POS_BME_PRS) - 0x80);
            Serial.println("");
          }
          break;
        default:
          break;
        }
      }
      return ret;
    }
    
    int initVersion(uint8_t mejorVer_, uint8_t minorVer_)
    {
      uint8_t mejorVerRom, minorVerRom, initFlg;
      mejorVerRom = EEPROM.read(EEP_POS_VERSION_MJR);
      minorVerRom = EEPROM.read(EEP_POS_VERSION_MNR);
      initFlg = EEPROM.read(EEP_POS_INIT_FLG);

      int version = mejorVer_;
      version = version * 100 + minorVer_;
      int versionRom = mejorVerRom;
      versionRom = versionRom * 100 + minorVerRom;

      if(initFlg != 1 || version > versionRom)
      {
        // initProcess.
        EEPROM.write(EEP_POS_VERSION_MJR, mejorVer_);
        EEPROM.write(EEP_POS_VERSION_MNR, minorVer_);
        EEPROM.write(EEP_POS_INIT_FLG, 1);
        
        if(version <= 101)
        {
          EEPROM.write(EEP_POS_CS_THRESH_1, 50);
          EEPROM.write(EEP_POS_CS_THRESH_2, 50);
          EEPROM.write(EEP_POS_CS_THRESH_3, 50);
          EEPROM.write(EEP_POS_CS_THRESH_4, 50);
          EEPROM.write(EEP_POS_BME_TMP, 0x80);
          EEPROM.write(EEP_POS_BME_HUM, 0x80);
          EEPROM.write(EEP_POS_BME_PRS, 0x80);
          EEPROM.write(EEP_POS_BME_COEF_TMP, 100);
          EEPROM.write(EEP_POS_BME_COEF_HUM, 100);
          EEPROM.write(EEP_POS_BME_COEF_PRS, 100);
        }
        Serial.print("init version ");
        Serial.println(version);
      }
      
      return 0;
      
    }

  private:

  };
}
