/*
 * @file irio.cpp
 * \~english
 * @brief 
 * \~japanese
 * @brief 赤外線 I/O の実装
 * \~ 
 * @author Masato Fujino, created on: May 23, 2019
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

#include <mutex>
#include <iostream>
#include <pigpio.h>
#include "tumbler/irio.h"

namespace tumbler{

IRProximitySensor& IRProximitySensor::getInstance(IRProximityDetectionCallback func, void* userdata)
{
	static IRProximitySensor instance(func, userdata);
	return instance;
}

IRProximitySensor::IRProximitySensor(IRProximityDetectionCallback func, void* userdata) :
		func_(func), userdata_(userdata), irio_(IRIO::getInstance()), sensitivity_(Sensitivity::high_)
{}

int IRProximitySensor::start()
{
	return start(IRProximitySensor::Sensitivity::high_);
}

int IRProximitySensor::start(IRProximitySensor::Sensitivity sensitivity)
{
	// 送信側を開始する
	if(irio_.proximityDetection()){
		return 2; // just ignore double start
	}
	char ack[8];
	int readlen = 0;
	ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
	{
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("IRLE",4);
		uint8_t subtype = 0;
		if(sensitivity == IRProximitySensor::Sensitivity::high_){
			subtype = 1;
		}else if(sensitivity == IRProximitySensor::Sensitivity::medium_){
			subtype = 2;
		}else if(sensitivity == IRProximitySensor::Sensitivity::low_){
			subtype = 3;
		}
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		readlen = subsystem.read(ack, 8);
	}
	if(readlen == 1 && ack[0] == '1'){
		irio_.startReceiver(func_, userdata_); // 受信側を開始する
		return 0; // OK
	}else{
		return 1; // NG
	}
}

int IRProximitySensor::stop()
{
	if(!irio_.proximityDetection()){
		return 2; // just ignore double stop
	}
	// 送信側を終了する
	char ack[8];
	int readlen = 0;
	ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
	{
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("IRLE",4);
		const uint8_t subtype = 0;
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		readlen = subsystem.read(ack, 8);
	}
	if(readlen == 1 && ack[0] == '1'){
		irio_.proximityDetection(false);
		irio_.stopReceiver(); // 受信側を終了する
		return 0; // OK
	}else{
		return 1; // NG
	}
}

IRSignalReceiver& IRSignalReceiver::getInstance(IRSignalReceiptCallback func, void* userdata)
{
	static IRSignalReceiver instance(func, userdata);
	return instance;
}

IRSignalReceiver::IRSignalReceiver(IRSignalReceiptCallback func, void* userdata) :
		func_(func), userdata_(userdata), irio_(IRIO::getInstance())
{}

int IRSignalReceiver::start()
{
	if(irio_.signalReceipt()){
		return 2;
	}
	return irio_.startReceiver(func_, userdata_);
}

int IRSignalReceiver::stop()
{
	if(!irio_.signalReceipt()){
		return 2;
	}
	irio_.signalReceipt(false);
	return irio_.stopReceiver();
}

void gpioCallbackBinder_(int gpio, int level, uint32_t tick, void* user)
{
	reinterpret_cast<IRIO*>(user)->commonGPIOCallback(gpio, level, tick);
}

void IRIO::hash(int old_val, int new_val)
{
   int val = 0;
   if(new_val < (old_val * 0.60)){
	   val = 13;
   }else if(old_val < (new_val * 0.60)){
	   val = 23;
   }else{
	   val = 2;
   }
   hash_ ^= val;
   hash_ *= 16777619;
}

void IRIO::commonGPIOCallback(int gpio, int level, uint32_t tick)
{
	//std::cout << "gpio=" << gpio << ",level=" << level << ",tick=" << tick << std::endl;
	if (level != PI_TIMEOUT){
		if (in_code_ == 0){
			in_code_ = 1;
			gpioSetWatchdog(receivergpio_, 5);
			hash_ = 2166136261U;
			edges_ = 1;
			t1_ = 0;
			t2_ = 0;
			t3_ = 0;
			t4_ = tick;
		}else{
			edges_++;
			t1_ = t2_;
			t2_ = t3_;
			t3_ = t4_;
			t4_ = tick;
			if (edges_ > 3){
				hash(t2_-t1_, t4_-t3_);
			}
		}
	}else{
		if (in_code_){
			in_code_ = 0;
			gpioSetWatchdog(receivergpio_, 0);
			if (edges_ > 12){
				if(flagSignalReceipt_.load()){
					funcSignalReceipt_(hash_, tick, userdataSignalReceipt_);
				}
			}else{
				if(flagProximityDetection_.load()){
					funcProximityDetection_(tick, userdataProximityDetection_);
				}
			}
		}


	}
}

int IRIO::startReceiver(IRProximityDetectionCallback func, void* userdata)
{
	int init = startReceiverC();
	if(init < 0){
		return init;
	}
	flagProximityDetection_.store(true);
	funcProximityDetection_ = func;
	userdataProximityDetection_ = userdata;
	return 0;
}

int IRIO::startReceiver(IRSignalReceiptCallback func, void* userdata)
{
	int init = startReceiverC();
	if(init < 0){
		return init;
	}
	flagSignalReceipt_.store(true);
	funcSignalReceipt_ = func;
	userdataSignalReceipt_ = userdata;
	return 0;
}


int IRIO::startReceiverC()
{
	std::lock_guard<std::mutex> lock(sslock_);
	if(flagSignalReceipt_.load() == false && flagProximityDetection_.load() == false){
		// どちらも動いていないときのみ
		int init = gpioInitialise();
		if(init < 0){
			return init;
		}
		gpioSetMode(receivergpio_, PI_INPUT);
		gpioSetAlertFuncEx(receivergpio_, gpioCallbackBinder_, (void *)this);
		return 0;
	}else{
		return 0;
	}
}

int IRIO::stopReceiver()
{
	std::lock_guard<std::mutex> lock(sslock_);
	if(flagSignalReceipt_.load() == false && flagProximityDetection_.load() == false){
		// どちらも動いていないときのみ
		gpioTerminate();
	}
	return 0;
}

}
