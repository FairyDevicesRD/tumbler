/*
 * @file irproximitysensor.cpp
 * \~english
 * @brief Example of proximity sensor with using IR I/O
 * \~japanese
 * @brief 赤外線 I/O を利用した近接センサーの実装例
 * \~ 
 * @author Masato Fujino, created on: May 28, 2019
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

#include <tumbler/buttons.h>
#include <tumbler/irio.h>
#include <iostream>
#include <unistd.h>

using namespace tumbler;

void ProximityDetectionFunc(uint32_t tick, void* userdata)
{
	std::cout << "Detection (tick=" << tick << ")" << std::endl;
}

int main(int argc, char** argv)
{
	// 赤外線 I/O を利用した近接センサーを開始します
	IRProximitySensor& sensor = IRProximitySensor::getInstance(ProximityDetectionFunc, nullptr);

	// 近接センサー出力を高とする、概ね 1 m 程度で反応する（デフォルト）
	sensor.start();

	// 近接センサー出力を中とする、概ね 60 cm 程度で反応する
	//sensor.start(IRProximitySensor::Sensitivity::medium_);

	// 近接センサー出力を低とする、概ね 30 cm 程度で反応する
	//sensor.start(IRProximitySensor::Sensitivity::low_);

	std::cout << "正面に障害物を設置するとコールバック関数が呼ばれます。このプログラムは 30 秒で終了します..." << std::endl;
	sleep(30); // 30 秒待ちます
	sensor.stop();
	return 0;
}

