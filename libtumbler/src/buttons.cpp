/*
 * @file buttons.cpp
 * \~english
 * @brief 
 * \~japanese
 * @brief タッチボタン制御クラスの実装
 * \~ 
 * @author Masato Fujino, created on: Mar 7, 2019
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

#include "tumbler/buttons.h"
#include <unistd.h>
#include <mutex>
#include <future>
#include <thread>
#include <iostream>
#include <syslog.h>

namespace tumbler{

int monitorAsync_(ButtonStateCallback func, std::atomic<bool>* stopflag)
{
	int errorno = 0;
	uint8_t buttonValue[4];
	std::vector<ButtonState> prevState(4);
	std::vector<ButtonState> currState(4);
	for(int i=0;i<4;++i){
		prevState[i] = ButtonState::none_;
	}
	while(stopflag->load() == false){
		// 通信（グローバルサブシステムロック）
		{
			ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
			std::lock_guard<std::mutex> lock(subsystem.global_lock_);
			const uint8_t subtype = 0;
			const uint8_t length = 0;
			char ack[1];
			int readlen = 0;
			subsystem.write("CAPR",4);
			subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
			subsystem.write(reinterpret_cast<const char*>(&length), 1);
			readlen = subsystem.read(ack, 1);
			if(readlen != 1){
				errorno = 1;
				break;
			}
			for(int i=0;i<4;++i){
				readlen = subsystem.read(reinterpret_cast<char*>(&buttonValue[i]), 1);
			}
		}
		if(errorno != 0){
			break;
		}
		// 判定
		for(int i=0;i<4;++i){
			unsigned short p = static_cast<unsigned short>(buttonValue[i]);
			if(60 < p){
				currState[i] = ButtonState::pushed_;
			}else{
				currState[i] = ButtonState::none_;
			}
		}
		// コールバック
		bool same = true;
		for(int i=0;i<4;++i){
			if(currState[i] != prevState[i]){
				same = false;
			}
		}
		if(same == false){
			func(currState);
			for(int i=0;i<4;++i){
				prevState[i] = currState[i];
			}
		}

		usleep(1000*100*2);
	}
	return errorno;
}

Buttons& Buttons::getInstance(ButtonStateCallback func)
{
	static Buttons instance(func);
	return instance;
}

Buttons::Buttons(ButtonStateCallback func) :
		subsystem_(ArduinoSubsystem::getInstance()),
		callback_(func),
		status_(false),
		stopflag_(false)
{}

void Buttons::start()
{
	status_ = true;
	monitor_ = std::async(std::launch::async, monitorAsync_, callback_, &stopflag_);
	syslog(LOG_INFO, "Button monitor started");
}

void Buttons::stop()
{
	status_ = false;
	stopflag_.store(true);
	int ret = monitor_.get();
	syslog(LOG_DEBUG,"Button monitor returns %d, stopped", ret);
}

}


