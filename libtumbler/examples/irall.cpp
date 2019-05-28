/*
 * @file irsignalreceiver.cpp
 * \~english
 * @brief Example of simultaneous use both of signal receiver and proximity sensor with using IR I/O function.
 * \~japanese
 * @brief 赤外線 I/O を近接センサー、外部信号受信機として共用する例
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

#include <tumbler/irio.h>
#include <iostream>
#include <unistd.h>

using namespace tumbler;

void ProximityDetectionFunc(uint32_t tick, void* userdata)
{
	std::cout << "Proximity Detection (tick=" << tick << ")" << std::endl;
}

void SignalReceiverFunc(uint32_t hash, uint32_t tick, void* userdata)
{
	std::cout << "Signal Receipt (hash=" << hash << ",tick=" << tick << ")" << std::endl;
}

int main(int argc, char** argv)
{
	// 赤外線 I/O を利用した近接センサーを開始します
	IRProximitySensor& sensor = IRProximitySensor::getInstance(ProximityDetectionFunc, nullptr);

	// 赤外線 I/O を利用して外部から赤外線信号を受信します
	IRSignalReceiver& receiver = IRSignalReceiver::getInstance(SignalReceiverFunc, nullptr);

	// 近接センサー出力を中とする、概ね 60 cm 程度で反応する
	sensor.start(IRProximitySensor::Sensitivity::medium_);

	// 赤外線信号の受信を開始します
	receiver.start();

	std::cout << "正面に障害物を設置、もしくは赤外線信号を与えると、それぞれコールバック関数が呼ばれます。このプログラムは 60 秒で終了します..." << std::endl;

	sleep(60); // 60 秒待ちます

	// 近接センサーを終了します
	sensor.stop();

	// 赤外線信号受信を驟雨漁します
	receiver.stop();
	return 0;
}
