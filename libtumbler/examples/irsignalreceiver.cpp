/*
 * @file irsignalreceiver.cpp
 * \~english
 * @brief Example of IR signal receiver from external signal source (i.e. IR remote controller) with using IR I/O function.
 * \~japanese
 * @brief 外部赤外線信号受信例（ELPA IRC-203T/メーカーコード1311での利用例を含む）
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

void SignalReceiverFunc(uint32_t hash, uint32_t tick, void* userdata)
{
	std::cout << "Signal Receipt (hash=" << hash << ",tick=" << tick << ")" << std::endl;

	// 入手性の良い ELPA IRC-203T リモコンを利用し、メーカーコード 1311 に設定した場合のハッシュ値の例
	// http://www.elpa.co.jp/product/av01/elpa1152.html
	//
	// 普通にリモコンボタンを押すだけで、複数回の信号が受信されることがあります。複数回の実行を行いたくない場合は、
	// tick 値を確認し、任意の短い期間の同一ハッシュ値のコールバック呼び出しを無視する実装を行うことができます。
	if(hash == 1787861072){
		std::cout << "電源 ON/OFF" << std::endl;
	}else if(hash == 2161514125){
		std::cout << "1" << std::endl;
	}else if(hash == 2415586405){
		std::cout << "2" << std::endl;
	}else if(hash == 419057658){
		std::cout << "3" << std::endl;
	}else if(hash == 1351526370){
		std::cout << "音量＋" << std::endl;
	}else if(hash == 2228258416){
		std::cout << "音量ー" << std::endl;
	}else if(hash == 3836380551){
		std::cout << "青" << std::endl;
	}else if(hash == 251580295){
		std::cout << "赤" << std::endl;
	}else if(hash == 3554650293){
		std::cout << "緑" << std::endl;
	}else if(hash == 329119533){
		std::cout << "黄" << std::endl;
	}
}

int main(int argc, char** argv)
{
	// 赤外線 I/O を利用して外部から赤外線信号を受信します
	IRSignalReceiver& receiver = IRSignalReceiver::getInstance(SignalReceiverFunc, nullptr);

	// 赤外線信号の受信を開始します
	receiver.start();

	std::cout << "赤外線信号を与えると、コールバック関数が呼ばれます。このプログラムは 60 秒で終了します..." << std::endl;

	sleep(60); // 60 秒待ちます

	// 赤外線信号受信を驟雨漁します
	receiver.stop();
	return 0;
}
