/*
 * @file IRLED.h
 * \~english
 * @brief 
 * \~japanese
 * @brief 赤外線 LED の制御
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
#ifndef ARDUINO_SKETCH_IRLED_H_
#define ARDUINO_SKETCH_IRLED_H_

namespace sonar{

class IRLED : public Module
{
public:
	IRLED() : Module("IRLED"){}
	int8_t init() override
	{
		pinMode(IR_GPIO_, OUTPUT);
		digitalWrite(IR_GPIO_, 0);
		return 0;
	}

	int8_t update(uint32_t frames) override
	{
		if(frames % 29 == 0){ // 0.2 sec に該当
			if(mode_ == 0){
				// 無出力
			}else if(mode_ == 1){
				staticSignal38kHz(); // 強
			}else if(mode_ == 2){
				staticSignal39kHz(); // 中
			}else if(mode_ == 3){
				staticSignal42kHz(); // 弱
			}
		}
		return 0;
	}

	int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length) override
	{
		if(strcmp(type, "IRLE") == 0){
			switch(subtype){
			case 0:
				{
					digitalWrite(IR_GPIO_, 0);
					mode_ = 0; // 消灯
				}
				break;
			case 1:
				{
					mode_ = 1;
				}
				break;
			case 2:
				{
					mode_ = 2;
				}
				break;
			case 3:
				{
					mode_ = 3;
				}
				break;
			default:
				break;
			}
		}
		return 0;
	}
private:

	void staticSignal42kHz()
	{
		for(int8_t i=0;i<24;++i){
			digitalWrite( IR_GPIO_, 1 );
			delayMicroseconds( 7 );
			digitalWrite( IR_GPIO_, 0 );
			delayMicroseconds( 8 );
		}
	}

	void staticSignal39kHz()
	{
		for(int8_t i=0;i<24;++i){
			digitalWrite( IR_GPIO_, 1 );
			delayMicroseconds( 10 );
			digitalWrite( IR_GPIO_, 0 );
			delayMicroseconds( 10 );
		}
	}

	void staticSignal38kHz()
	{
		for(int8_t i=0;i<24;++i){
			/*
			PORTC = _BV(1); // 0.18 us
			delayMicroseconds(13); // 13 us
			PORTC = ~_BV(1); // 0.18 us
			delayMicroseconds(13); // 13 us
			// sum = 26.36 us : 37.9kHz
			 */
			digitalWrite( IR_GPIO_, 1 );   // 44cycle = 2.75us
			delayMicroseconds( 10 );
			digitalWrite( IR_GPIO_, 0 );
			delayMicroseconds( 11 );
			// sum = 26.5 us :
			// 点灯 = 12.75 us、消灯 13.75 us、サイクルタイム 26.5 us
		} //600 us
	}

	const int IR_GPIO_ = 15;
	int8_t mode_ = 0;
};
}

#endif /* ARDUINO_SKETCH_IRLED_H_ */
