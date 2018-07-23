/*
 * @file touch.cpp
 * @brief タッチセンサー制御クラスの実装
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */

#include "tumbler/touch.h"
#include <iostream>
#include <unistd.h>
#include <mutex>
#include <future>
#include <thread>
#include <string.h>

namespace tumbler{

#define TOUCH_STATE_VAL_OFF    0
#define TOUCH_STATE_VAL_OFFS   1
#define TOUCH_STATE_VAL_SHORT  2
#define TOUCH_STATE_VAL_SL     3
#define TOUCH_STATE_VAL_LONG   4
#define TOUCH_STATE_VAL_SOFF   5
#define TOUCH_STATE_VAL_LOFF   7

#define TOUCH_STATE_SYM_OFF    ("OO")
#define TOUCH_STATE_SYM_OFFS   ("OS")
#define TOUCH_STATE_SYM_SHORT  ("SS")
#define TOUCH_STATE_SYM_SL     ("SL")
#define TOUCH_STATE_SYM_LONG   ("LL")
#define TOUCH_STATE_SYM_SOFF   ("SO")
#define TOUCH_STATE_SYM_LOFF   ("LO")
	
	
TouchSensor& TouchSensor::getInstance()
{
	static TouchSensor instance;
	return instance;
}

/**
 * @brief シリアル要求：タッチセンサー値取得.
 * @ret   正常に更新できれば、1
 */
static int TouchSensor_getStateImpl_()
{
	{
		std::cout << "--Get Touch State--" << std::endl;
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();

		// シリアル受信スレッドが動作している場合は抜ける. 
		if(subsystem.getCallBackState()) return 0;

		std::lock_guard<std::mutex> lock(subsystem.global_lock_);

		subsystem.write("TSNS",4);
		const uint8_t subtype = 0;
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.flush();

		TouchSensor &touch = TouchSensor::getInstance();
		char readBuf[20];
		memset(readBuf, 0, 20);
		int len = subsystem.readline(readBuf, 20);
		touch.update(readBuf, len);
	}
	return 0;
}

/**
 * @brief シリアル要求：タッチセンサー感度取得.
 * @ret   正常に更新できれば、1
 */
static int TouchSensor_getThreshImpl_()
{
	int ret = 0;
	{
		std::cout << "--Get Thresh Value--" << std::endl;
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);

		subsystem.write("TSNS",4);
		const uint8_t subtype = 1;
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.flush();
	}

	ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
	TouchSensor& touch = TouchSensor::getInstance();
	if(subsystem.getCallBackState())
	{
		// シリアル受信スレッドが動作している場合は更新フラグが立つのを待つ。
		int breakTime = 30;
		while(touch.isUpdate() == 0 && breakTime-- > 0){ usleep(10000);}
		ret = touch.isUpdate();
	} else {
		// シリアル受信スレッドが停止している場合は直接応答を取得する。
		char readBuf[64];
		memset(readBuf, 0, 64);
		int len  = subsystem.readline(readBuf, 63);
		ret = touch.update(readBuf, len);
	}
	
	return ret;
}

/**
 * @brief シリアル要求：タッチセンサー計測値取得.
 * @ret   正常に更新できれば、1
 */
static int TouchSensor_getChargeImpl_()
{
	int ret = 0;
	{
		std::cout << "--Get Charge Value--" << std::endl;
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);

		subsystem.write("TSNS",4);
		const uint8_t subtype = 9;
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.flush();
	}

	ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
	TouchSensor& touch = TouchSensor::getInstance();
	if(subsystem.getCallBackState())
	{
		// シリアル受信スレッドが動作している場合は更新フラグが立つのを待つ。
		int breakTime = 30;
		while(touch.isUpdate() == 0 && breakTime-- > 0){ usleep(10000);}
		ret = touch.isUpdate();
	} else {
		// シリアル受信スレッドが停止している場合は直接応答を取得する。
		char readBuf[64];
		memset(readBuf, 0, 64);
		int len  = subsystem.readline(readBuf, 63);
		ret = touch.update(readBuf, len);
	}
	
	return ret; 
}

/**
 * @brief シリアル要求：タッチセンサー感度更新要求.
 */
static int TouchSensor_setThreshImpl_(uint8_t btn1, uint8_t btn2, uint8_t btn3, uint8_t btn4, bool eep)
{
	{
		std::cout << "--SetThresh--" << std::endl;
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("TSNS",4);
	  const uint8_t subtype = (eep ? 3 : 2);
		const uint8_t length  = 4;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.write(reinterpret_cast<const char*>(&btn1), 1);
		subsystem.write(reinterpret_cast<const char*>(&btn2), 1);
		subsystem.write(reinterpret_cast<const char*>(&btn3), 1);
		subsystem.write(reinterpret_cast<const char*>(&btn4), 1);
		subsystem.flush();
	}
	
	return 0; // OK
}


/**
 * @brief シリアル要求：タッチセンサー値通知機能ON/OFF要求.
 */
static int TouchSensor_setReportImpl_(uint8_t onoff)
{
	{
		std::cout << "--SetReport--" << std::endl;
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("TSNS",4);
		const uint8_t subtype = 4;
		const uint8_t length  = 1;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.write(reinterpret_cast<const char*>(&onoff), 1);
		subsystem.flush();
	}
	
	return 0; // OK
}


TouchSensor::TouchSensor() :
		subsystem_(ArduinoSubsystem::getInstance())
{
	updateFlg_ = 0;
}

/**
 * @brief シリアル応答：各種更新処理.
 */
int TouchSensor::update(char* data, int size)
{ 
	char tmp[10];
	char *target = data;
	if(data[0] == 'T' && data[1] == 'S'){
		//タッチ応答フォーマットは「TS0 1 2 3 」
		for(int i = 0,j = 2; i < 4; ++i,j+=2){
			stateVal_[i] = data[j] - '0';
		}
	}
	else if(data[0] == 'T' && data[1] == 'T'){
		//感度応答フォーマットは「TT10 20 30 40 」
		target += 2;
		for(int i = 0; i < 4; ++i){
			for(int j = 0; j < 4;j++){
				if(*target != ' '){
					tmp[j] = *target;
					target++;
				}
				else{
					tmp[j] = '\0';
					threshVal_[i] = atoi(tmp);
					target++;
					break;
				}
			}
		}
		updateFlg_ = 1;
	}
	else if(data[0] == 'T' && data[1] == 'V'){
		//測定値応答フォーマットは「TV10 20 30 40 」
		target += 2;
		for(int i = 0; i < 4; ++i){
			for(int j = 0; j < 4;j++){
				if(*target != ' '){
					tmp[j] = *target;
					target++;
				}
				else{
					tmp[j] = '\0';
					chargeVal_[i] = atoi(tmp);
					target++;
					break;
				}
			}
		}
		updateFlg_ = 1;
	}
	return updateFlg_;
}



/**
 * @brief タッチセンサー状態取得。
 */
int TouchSensor::readState(bool async)
{
	updateFlg_ = 0;
	if(async){
		touchAsync_ = std::async(std::launch::async, TouchSensor_getStateImpl_);
		return 0;
	}else{
		return TouchSensor_getStateImpl_();
	}
}

int TouchSensor::getStateNow(int target)
{
	int ret = TOUCH_STATE_OFF;
	switch(stateVal_[target])
	{
	case 0: case 5: case 7:  // 停止.
		ret = TOUCH_STATE_OFF;  break;
	case 1: case 2:          // 短押し.
		ret = TOUCH_STATE_SHORT; break;
	case 3: case 4:          // 長押し.
		ret = TOUCH_STATE_LONG;  break;
	default:
		break;
	}
	return ret;
}
int TouchSensor::getStateVal(int target)
{
	return stateVal_[target];
}
int* TouchSensor::getStatePtr()
{
	return stateVal_;
}

/**
 * @brief タッチセンサー感度取得.
 */
int TouchSensor::readThresh(bool async)
{
	updateFlg_ = 0;
	if(async){
		touchAsync_ = std::async(std::launch::async, TouchSensor_getThreshImpl_);
		return 0;
	}else{
		return TouchSensor_getThreshImpl_();
	}
}

int TouchSensor::getThreshVal(int target)
{
	return threshVal_[target];
}
int* TouchSensor::getThreshPtr()
{
	return threshVal_;
}

/**
 * @brief タッチセンサー計測値取得.
 */
int TouchSensor::readCharge(bool async)
{
	updateFlg_ = 0;
	if(async){
		touchAsync_ = std::async(std::launch::async, TouchSensor_getChargeImpl_);
		return 0;
	}else{
		return TouchSensor_getChargeImpl_();
	}
}

int TouchSensor::getChargeVal(int target)
{
	return chargeVal_[target];
}
int* TouchSensor::getChargePtr()
{
	return chargeVal_;
}

/**
 * @brief タッチセンサー感度設定.
 */
int TouchSensor::setThreshVal(bool async, uint8_t btn1, uint8_t btn2, uint8_t btn3, uint8_t btn4, bool eep)
{
	if(async){
		touchAsync_ = std::async(std::launch::async, TouchSensor_setThreshImpl_, btn1, btn2, btn3, btn4, eep);
		return 0;
	}else{
		return TouchSensor_setThreshImpl_(btn1, btn2, btn3, btn4, eep);
	}
}

/**
 * @brief タッチセンサー感度設定(0.0～1.0).
 */
int TouchSensor::setThreshPer(bool async, float btn1, float btn2, float btn3, float btn4, bool eep)
{
	char btn[4];
	btn[0] = GetTouchThresh(btn1); if(btn[0] < 0) btn[0] = TOUCH_THRESH_MAX;
	btn[1] = GetTouchThresh(btn2); if(btn[1] < 0) btn[1] = TOUCH_THRESH_MAX;
	btn[2] = GetTouchThresh(btn3); if(btn[2] < 0) btn[2] = TOUCH_THRESH_MAX;
	btn[3] = GetTouchThresh(btn4); if(btn[3] < 0) btn[3] = TOUCH_THRESH_MAX;
	return setThreshVal(async, (uint8_t)btn[0], (uint8_t)btn[1], (uint8_t)btn[2], (uint8_t)btn[3], eep);
}

/**
 * @brief タッチセンサー値通知機能ON/OFF変更.
 */
int TouchSensor::setReport(bool async, uint8_t onoff)
{
	if(async){
		touchAsync_ = std::async(std::launch::async, TouchSensor_setReportImpl_, onoff);
		return 0;
	}else{
		return TouchSensor_setReportImpl_(onoff);
	}
}

/**
 * @brief タッチセンサーのコールバック関数を登録する
 */
int TouchSensor::setCallBackFnc(void (*callbackFnc)(char*,int))
{
	ArduinoSubsystem& subsystem_ = ArduinoSubsystem::getInstance();
	subsystem_.startCallBack(callbackFnc);
	return 0;
}


}

