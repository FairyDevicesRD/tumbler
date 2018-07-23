/*
 * @file serial_test.cpp
 * @brief touch.h の連続受信単体試験
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/22 
 */

#include <iostream>
#include <unistd.h>
#include "tumbler/tumbler.h"
#include "tumbler/ledring.h"
#include "tumbler/touch.h"
#include "tumbler/bme.h"


static void callbackfnc(char* command, int len)
{
	if(len < 2){
		std::cout << "callback:" << command << std::endl;
		return;
	}
	
	// 先頭2文字で判定.
	if(command[0] == 'T' && command[1] == 'S'){
		std::cout << "[Touch State]" << command << std::endl;
	}
	else if(command[0] == 'T' && command[1] == 'T'){
		std::cout << "[Touch Thresh]" << command << std::endl;
	}
	else if(command[0] == 'T' && command[1] == 'V'){
		std::cout << "[Touch Value]" << command << std::endl;
	}
	else{
		std::cout << "[LOG]" << command << std::endl;
	}
}



int main(int argc, char** argv)
{
	using namespace tumbler;
	{
		// タッチセンサーのコールバック関数を設定する。
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.setCallBackFnc(callbackfnc);
	}
	{
		// 感度を変更
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.setThreshVal(false,50,50,50,50,false); // 同期
		sensor.readThresh(false); // 同期
		sleep(1);
	}
	{
		// 常時受信モードに設定
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.setReport(false,1); // 同期
		sleep(1);
	}
	{
		// LEDリングをタッチに合わせて点灯させる.
		LEDRing& ring = LEDRing::getInstance();
		ring.touch(false);
		sleep(1);
	}
	{
		BMEControl& bme = BMEControl::getInstance();
		bme.setAdjust(false,100,-20,100,25,100,0,false);
		sleep(1);
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.readThresh(false); // 同期
		sleep(1);
		for(int i = 0; i < 30; ++i)
		{
			// BMEの計測値を取得してみる
			bme.get(false);
			std::cout << "Temp / Hum / Press" << std::endl;
			std::cout << bme.getTemp() << " / ";
			std::cout << bme.getHum() << " / ";
			std::cout << bme.getPress() << std::endl;

			// タッチセンサー計測値を取得
			sensor.readCharge(false); // 同期
			//sleep(1);
			int * tmpState = sensor.getStatePtr();
			int * tmpThresh = sensor.getThreshPtr();
			int * tmpCharge = sensor.getChargePtr();
			std::cout << "State/Thresh/Charge" << std::endl;
			for(int j = 0; j < 4; ++j){
				std::cout << tmpState[j] << " ";
			}
			std::cout << " / ";
			for(int j = 0; j < 4; ++j){
				std::cout << tmpThresh[j] << " ";
			}
			std::cout << " / ";
			for(int j = 0; j < 4; ++j){
				std::cout << tmpCharge[j] << " ";
			}
			std::cout << std::endl;
			sleep(1);
		}
	}

	{
		// LEDリングをリセット.
		LEDRing& ring = LEDRing::getInstance();
		ring.reset(true); // 非同期でリセットし終了
	}	
	{
		// 常時受信モードをOFFに設定
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.setReport(false,0); // 同期
		sleep(1);
	}

	return 0;
}


