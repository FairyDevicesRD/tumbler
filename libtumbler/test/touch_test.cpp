/*
 * @file touch_test.cpp
 * @brief touch.h の単体試験
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/22 
 */

#include <iostream>
#include <unistd.h>
#include "tumbler/tumbler.h"
#include "tumbler/ledring.h"
#include "tumbler/touch.h"

using namespace tumbler;

static void callbackfnc(char* command, int len)
{
	if(len < 2){
		std::cout << "callback:" << command << std::endl;
		return;
	}
	if(0){	
		// 先頭2文字で判定出来る.ただし実際にはシリアル受信時に解析済み.
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

	// ボタンの状態に合わせてLEDを点灯させてみる.
	LED background(0,10,0);
	Frame frame(background);
	TouchSensor& sensor = TouchSensor::getInstance();

	LEDRing& ring = LEDRing::getInstance();
	uint8_t btn = sensor.getStateNow(0);
	if(btn == TOUCH_STATE_OFF){
		ring.setOne(true,16, 0,0,0);
	} else if(btn == TOUCH_STATE_SHORT){
		ring.setOne(true,16, 250,250,0);
	} else if(btn == TOUCH_STATE_LONG){
		ring.setOne(true,16, 0,0,250);
	} else{
		ring.setOne(true,16, 250,0,0);
	}
		
	btn = sensor.getStateNow(1);
	if(btn == TOUCH_STATE_OFF){
		ring.setOne(true,0, 0,0,0);
	} else if(btn == TOUCH_STATE_SHORT){
		ring.setOne(true,0, 250,250,0);
	} else if(btn == TOUCH_STATE_LONG){
		ring.setOne(true,0, 0,0,250);
	} else{
		ring.setOne(true,0, 250,0,0);
	}

	btn = sensor.getStateNow(2);
	if(btn == TOUCH_STATE_OFF){
		ring.setOne(true,2, 0,0,0);
	} else if(btn == TOUCH_STATE_SHORT){
		ring.setOne(true,2, 250,250,0);
	} else if(btn == TOUCH_STATE_LONG){
		ring.setOne(true,2, 0,0,250);
	} else{
		ring.setOne(true,2, 250,0,0);
	}

	btn = sensor.getStateNow(3);
	if(btn == TOUCH_STATE_OFF){
		ring.setOne(true,4, 0,0,0);
	} else if(btn == TOUCH_STATE_SHORT){
		ring.setOne(true,4, 250,250,0);
	} else if(btn == TOUCH_STATE_LONG){
		ring.setOne(true,4, 0,0,250);
	} else{
		ring.setOne(true,4, 250,0,0);
	}

}


int main(int argc, char** argv)
{
	{
		// 感度を変更
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.setThreshVal(false,50,50,50,50,false); // 同期
		sensor.setReport(false,1);
		sleep(1);
	}
	if(0)
	{
			// タッチ点灯
		LEDRing& ring = LEDRing::getInstance();
		ring.touch(false); // 同期
		sleep(1);
		std::cout << "Please touch any button." << std::endl;
	}
	else
	{
		// タッチセンサーのコールバック関数を設定する。
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.setCallBackFnc(callbackfnc);
	}
  
	{
		// 現在の感度を取得
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.readThresh(false); // 同期
			sleep(1);
		int *tmpVal = sensor.getThreshPtr();
		std::cout << "thresh:" << tmpVal[0] << " " << tmpVal[1] << " "<< tmpVal[2] << " "<< tmpVal[3] << std::endl; 
	}
	{
		// 現在のタッチ状態と計測値を取得
		TouchSensor& sensor = TouchSensor::getInstance();
		for(int i = 0; i < 10; ++i)
		{
			sleep(3);
		}
		if(0)
		{
			sensor.readState(false); // 同期
			sleep(1);
			int *tmpVal = sensor.getStatePtr();
			std::cout << "State:" << tmpVal[0] << " " << tmpVal[1] << " "<< tmpVal[2] << " "<< tmpVal[3] << std::endl; 
			sensor.readCharge(false); // 同期
			sleep(1);
			tmpVal = sensor.getChargePtr();
			std::cout << "Value:" << tmpVal[0] << " " << tmpVal[1] << " "<< tmpVal[2] << " "<< tmpVal[3] << std::endl; 
		}
	}
	{
		// 感度を変更(EEPROM保存)
		TouchSensor& sensor = TouchSensor::getInstance();
		sensor.setThreshVal(false,50,50,50,50,true); // 同期
		sleep(3);
	}

	LEDRing& ring = LEDRing::getInstance();
	ring.reset(false); // 非同期でリセットし終了
	return 0;
}


