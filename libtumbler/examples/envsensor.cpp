/*
 * @file envsensor.cpp
 * \~english
 * @brief 
 * \~japanese
 * @brief 
 * \~ 
 * @author Masato Fujino, created on: May 14, 2019
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


#include "tumbler/tumbler.h"
#include "tumbler/envsensor.h"
#include <iostream>

using namespace tumbler;

int main(int argc, char** argv)
{
	EnvSensor& sensor = EnvSensor::getInstance();
	std::cout << "Temperature: " << sensor.temperature() << " C" << std::endl;
	std::cout << "Humidity: " << sensor.humidity() << " %" << std::endl;
	std::cout << "Pressure: " << sensor.pressure() << " hPa" << std::endl;
	return 0;
}
