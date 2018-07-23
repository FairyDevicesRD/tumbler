namespace sonar
{
  class BMEControl : public Module
  {
  public:
    BMEControl() :
      Module("BMEControl"),
      temperature_(0.),
      humidity_(0.),
      pressure_(0.),
      coef_temperature_(1.0),
      coef_humidity_(1.0),
      coef_pressure_(1.0),
      adj_temperature_(0),
      adj_humidity_(0),
      adj_pressure_(0)
    {}

    int8_t init() override
    {
      Serial.print("BmeControl initializing ... ");
      // bme280_.setMode(0x76, 4, 4, 4, 3, 5, 0, 0);
      bme280_.setMode(0x76, 2, 2, 2, 3, 5, 0, 0);
      bme280_.readTrim();
      if(EEPROM.read(EEP_POS_INIT_FLG) == 1){
        coef_temperature_ = (double)((uint8_t)EEPROM.read(EEP_POS_BME_COEF_TMP)) / 100.0;
        coef_humidity_    = (double)((uint8_t)EEPROM.read(EEP_POS_BME_COEF_HUM)) / 100.0;
        coef_pressure_    = (double)((uint8_t)EEPROM.read(EEP_POS_BME_COEF_PRS)) / 100.0;
        adj_temperature_  = EEPROM.read(EEP_POS_BME_TMP) - 0x80;
        adj_humidity_     = EEPROM.read(EEP_POS_BME_HUM) - 0x80;
        adj_pressure_     = EEPROM.read(EEP_POS_BME_PRS) - 0x80;
      }

      Serial.println(" [ OK ]");
      return 0;
    }

    int8_t update(uint32_t frames) override
    {
      int8_t ret = 0;
      if(frames % constants::fps_ == 0){ // 1 FPS
        bme280_.readData(&temperature_,&pressure_,&humidity_);
        temperature_ = temperature_ * coef_temperature_ + adj_temperature_;
        humidity_ = humidity_ * coef_humidity_ + adj_humidity_;
        pressure_ = pressure_ * coef_pressure_ + adj_pressure_;
        ret = 1;
      }

      return ret;
    }

    int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length) override
    {
      if(strcmp(type, "BMES") == 0){
        switch(subtype){
        case 2:    // Save adjust value for EEPROM.
        {
          if(length < 6){}
          else{
            EEPROM.write(EEP_POS_BME_COEF_TMP, body[0]);
            EEPROM.write(EEP_POS_BME_COEF_HUM, body[2]);
            EEPROM.write(EEP_POS_BME_COEF_PRS, body[4]);
            EEPROM.write(EEP_POS_BME_TMP, body[1] + 0x80);
            EEPROM.write(EEP_POS_BME_HUM, body[3] + 0x80);
            EEPROM.write(EEP_POS_BME_PRS, body[5] + 0x80);
          }
        }
        // no break.
        case 1:    // Set adjust value.
        {
          if(length < 6){}
          else{
            coef_temperature_ = (double)((uint8_t)body[0]) / 100.0;
            coef_humidity_    = (double)((uint8_t)body[2]) / 100.0;
            coef_pressure_    = (double)((uint8_t)body[4]) / 100.0;
            adj_temperature_  = body[1];
            adj_humidity_     = body[3];
            adj_pressure_     = body[5];
          }
          Serial.println("bme adjust update.");
          break;
        }
        case 0:
        default:
          {
            // get value.
            Serial.print("BM:T=");
            Serial.print(temperature_);
            Serial.print(" H=");
            Serial.print(humidity_);
            Serial.print(" P=");
            Serial.println(pressure_);
            break;
          }
        }
      }
      return 0;
    }

  private:
    SSCI_BME280 bme280_;
    double temperature_;
    double humidity_;
    double pressure_;
    
    double coef_temperature_;
    double coef_humidity_;
    double coef_pressure_;
    int8_t adj_temperature_;
    int8_t adj_humidity_;
    int8_t adj_pressure_;
  };
}
