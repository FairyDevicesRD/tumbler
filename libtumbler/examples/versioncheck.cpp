/*
 * @file versioncheck.cpp
 * \~english
 * @brief Example program for checking arduino skech version
 * \~japanese
 * @brief Arduino スケッチのバージョン確認
 * \~ 
 * @author Masato Fujino, created on: Apr 16, 2019
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

#include <iostream>
#include <unistd.h>
#include "tumbler/tumbler.h"

using namespace tumbler;

int main(int argc, char** argv)
{
	ArduinoSubsystem &system = ArduinoSubsystem::getInstance();
	int version = system.sketchVersion();
	if(version < 0){
		std::cout << "Error: Version check failed: " << version << std::endl;
		return 1;
	}else{
		std::cout << "Sketch Version: " << version << std::endl;
		return 0;
	}
}
