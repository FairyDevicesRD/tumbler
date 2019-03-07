/*
 * @file TouchButtons.h
 * \~english
 * @brief Contoller for 4 touch buttons
 * \~japanese
 * @brief ４つ組のタッチボタン制御
 * \~ 
 * @author Masato Fujino, created on: Mar 6, 2019
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
#ifndef ARDUINO_SKETCH_TOUCHBUTTONS_H_
#define ARDUINO_SKETCH_TOUCHBUTTONS_H_

namespace sonar{

class TouchButtons : public Module
{
public:
	TouchButtons() : Module("TouchButtons"), vsc_(0)
	{
		for(int i=0;i<4;++i){
			sv1_[i] = 0;
		}
	}

	int8_t init() override
	{
		c_[0] = CapacitiveSensor(3,4);
		c_[1] = CapacitiveSensor(5,6);
		c_[2] = CapacitiveSensor(7,8);
		c_[3] = CapacitiveSensor(9,10);
		return 0;
	}

	int8_t update(uint32_t frames) override
	{
		if(frames % 80 == 0){
			if(vsc_ == 2){
				for(int i=0;i<4;++i){
					sv2_[i] = sv1_[i];
					sv1_[i] = 0;
				}
				vsc_ = 0;
				//Serial.println(sv2_[3]);
			}
			for(int i=0;i<4;++i){
				sv1_[i] += c_[i].capacitiveSensor(40);
			}
			vsc_++;
		}
		return 0;
	}

	int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length) override
	{
		if(strcmp(type, "CAPR") == 0){
			switch(subtype){
			case 0:
				{
					for(int i=0;i<4;i++){
						if(255 < sv2_[i]){
							sv2_[i] = 255;
						}
						uint8_t v = static_cast<uint8_t>(sv2_[i]);
						Serial.write(v);
					}
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
	CapacitiveSensor c_[4];
	uint16_t sv1_[4];
	uint16_t sv2_[4];
	int8_t vsc_;
};

}


#endif /* ARDUINO_SKETCH_TOUCHBUTTONS_H_ */
