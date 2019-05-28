/*
 * @file LightSensor.hpp
 * \~english
 * @brief Controller for light sensor (LTR-329ALS)
 * \~japanese
 * @brief 光センサーの制御
 * \~ 
 * @author Masato Fujino, created on: May 16, 2019
 * @copyright Copyright 2019 Fairy Devices Inc. http://www.fairydevices.jp/
 * @copyright Apache License, Version 2.0
 *
 * Copyright 2019 Fairy Devices Inc. http://www.fairydevices.jp/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ARDUINO_SKETCH_LIGHTSENSOR_H_
#define ARDUINO_SKETCH_LIGHTSENSOR_H_

namespace sonar{

/**
 * @class LightSensor
 * @brief 光センサー（LTR-329ALS）を表すクラス
 * @see ハードウェア仕様書 http://optoelectronics.liteon.com/upload/download/DS86-2014-0006/LTR-329ALS-01_DS_V1.pdf
 */
class LightSensor : public Module
{
public:
	LightSensor() : Module("LightSensor"){}

	int8_t init() override
	{
		// 仕様書 p14
		Wire.beginTransmission(I2C_ADDR_);
		Wire.write(0x80);
		// gain x1 : 0x01
		// gain x8 : 0x0D
		Wire.write(0x19); //gain x48, active mode
		Wire.endTransmission();

		// 仕様書 p15
		// 赤外線 LED との干渉を防ぐために測定値の安定性を犠牲として測定間隔を最小とした
		Wire.beginTransmission(I2C_ADDR_);
		Wire.write(0x85);
		Wire.write(0x08); // Integration time = 50ms, measurement time=50ms
		Wire.endTransmission();

		// 赤外線 LED を消灯する
		pinMode(15, OUTPUT);
		digitalWrite(15, 0);
		return 0;
	}

	int8_t update(uint32_t frames) override
	{
		if(frames % 15 == 0){
			// チャンネル 0 のみの読み出し
			byte msb = 0;
			byte lsb = 0;
			Wire.beginTransmission(I2C_ADDR_);
			Wire.write(0x8A); //low
			Wire.endTransmission();
			Wire.requestFrom((uint8_t)I2C_ADDR_, (uint8_t)1);
			//delay(1);
			if(Wire.available())
				lsb = Wire.read();

			Wire.beginTransmission(I2C_ADDR_);
			Wire.write(0x8B); //high
			Wire.endTransmission();
			Wire.requestFrom((uint8_t)I2C_ADDR_, (uint8_t)1);
			//delay(1);
			if(Wire.available())
			    msb = Wire.read();

			uint16_t rawlightlux = (msb<<8) | lsb;
			if(rawlightlux != 0){
				// 赤外線 LED によるセンサー飽和状態の場合に無視
				// @TODO TODO ALS_STATUS レジスタ（仕様書18p）を確認すれば飽和状態が直接検出できるか？
				samples_[sample_counter_++] = rawlightlux;
				if(sample_counter_ == 8){
					sample_counter_ = 0;
					uint16_t min_sample = 65535;
					for(int8_t i=0;i<8;++i){
						if(samples_[i] < min_sample){
							min_sample = samples_[i];
						}
					}
					lsb_ = min_sample & 0xFF;
					msb_ = min_sample >> 8;
				}
				return 0;
			}else{
				return 1;
			}
		}
		return 0;
	}

	int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length) override
	{
		if(strcmp(type, "LTRD") == 0){
			switch(subtype){
			case 0:
				{
					Serial.write(lsb_);
					Serial.write(msb_);
					Serial.flush();
				}
				break;
			case 1:
				{
				}
				break;
			case 2:
				{
				}
				break;
			}
		}
		return 0;
	}

private:
	const int I2C_ADDR_ = 0x29;
	uint16_t samples_[8];
	int8_t sample_counter_ = 0;
	byte msb_ = 0;
	byte lsb_ = 0;
};

}

#endif /* ARDUINO_SKETCH_LIGHTSENSOR_H_ */
