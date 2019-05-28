/*
 * @file lightsensor.cpp
 * \~english
 * @brief 
 * \~japanese
 * @brief 
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
#ifndef LIBTUMBLER_SRC_LIGHTSENSOR_CPP_
#define LIBTUMBLER_SRC_LIGHTSENSOR_CPP_

#include "tumbler/tumbler.h"
#include "tumbler/lightsensor.h"

namespace tumbler{

LightSensor& LightSensor::getInstance()
{
	static LightSensor instance;
	return instance;
}

unsigned int LightSensor::light()
{
	char lsb = 0;
	char msb = 0;
	ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
	{
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		const uint8_t subtype = 0;
		const uint8_t length = 0;
		char ack[1];
		int readlen = 0;
		subsystem.write("LTRD",4);
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		readlen = subsystem.read(ack, 1);
		if(readlen != 1){
			return 0.0F; // TODO エラーステート管理
		}
		subsystem.read(&lsb, 1);
		subsystem.read(&msb, 1);
	}
	uint16_t lightlux = (msb<<8) | lsb;
	return static_cast<unsigned int>(lightlux);
}

LightSensor::LightSensor(){}

};

#endif /* LIBTUMBLER_SRC_LIGHTSENSOR_CPP_ */
