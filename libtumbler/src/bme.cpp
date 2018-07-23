/*
 * @file bme.cpp
 * @brief BME 環境センサー制御クラスの実装
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */

#include "tumbler/bme.h"
#include <iostream>
#include <unistd.h>
#include <mutex>
#include <future>
#include <thread>
#include <memory>
#include <string.h>

namespace tumbler{

/**
 * @brief シングルトン取得.
 */
BMEControl& BMEControl::getInstance()
{
	static BMEControl instance;
	return instance;
}

/**
 * @brief シリアル要求：環境センサー値取得.
 * @ret   正常に更新できれば、1
 */
static int BMEControl_getImpl_()
{
	int ret = 0;
	{
		std::cout << "--Get BME Value--" << std::endl;
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);

		subsystem.write("BMES",4);
		const uint8_t subtype = 0;
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.flush();
	}

	ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
	BMEControl& bme = BMEControl::getInstance();
	if(subsystem.getCallBackState()){
		// シリアル受信スレッドが動作している場合は更新フラグが立つのを待つ。
		int breakTime = 50;
		while(bme.isUpdate() == 0 && breakTime-- > 0){ usleep(10000);}
		ret = bme.isUpdate();
	}else{
		// シリアル受信スレッドが停止している場合は直接応答を取得する。
		char tmp[80];
		int len = subsystem.readline(tmp, 80);
		bme.update(tmp, len);
		ret = bme.isUpdate();
	}
	return ret;
}
	
int BMEControl_setAdjImpl_(uint8_t tmpCoef_, char tmp_, uint8_t humCoef_, char hum_, uint8_t pressCoef_, char press_, bool eep_)
{
	int ret = 0;
	{
		std::cout << "--Set BME Adjust--" << std::endl;
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);

		subsystem.write("BMES",4);
		const uint8_t subtype = (eep_ ? 2 : 1);
		const uint8_t length  = 6;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.write(reinterpret_cast<const char*>(&tmpCoef_), 1);
		subsystem.write(reinterpret_cast<const char*>(&tmp_), 1);
		subsystem.write(reinterpret_cast<const char*>(&humCoef_), 1);
		subsystem.write(reinterpret_cast<const char*>(&hum_), 1);
		subsystem.write(reinterpret_cast<const char*>(&pressCoef_), 1);
		subsystem.write(reinterpret_cast<const char*>(&press_), 1);
		subsystem.flush();
	}
	return ret;
}

/**
 * @brief シリアル応答：環境センサー値更新.
 */
void BMEControl::update(char* data, int datasize) 
{
	//応答フォーマットは「BM:T=20.22 H=21.22 P=1024.15」
	char tmp[10];
	char *target = data;
	std::cout << "BME update: " << target << std::endl;

	if(*target == 'B' && *(target+1) == 'M'){
		target += 5;	// 「BM:T=」
		// 温度.
		for(int i = 0; i < 10; ++i){
			if(*target != ' '){
				tmp[i] = *target; target++;
			}
			else{
				tmp[i] = '\0';
				setTemp(atof(tmp));
				break;
			}
		}
		// 湿度
		target += 3;	// 「 H=」
		for(int i = 0; i < 10; ++i){
			if(*target != ' '){
				tmp[i] = *target; target++;
			}
			else{
				tmp[i] = '\0';
				setHum(atof(tmp));
				break;
			}
		}
		// 気圧
		target += 3;	// 「 P=」
		for(int i = 0; i < 10; ++i){
			if(*target != ' ' && *target != '\n'){
				tmp[i] = *target; target++;
			}
			else{
				tmp[i] = '\0';
				setPress(atof(tmp));
				break;
			}
		}
		updateflg_ = 1;
	}
	else{
		std::cout << "BME update Error!! : " << target << std::endl;
	}
}

/**
 * @brief コンストラクタ
 */
BMEControl::BMEControl() :
		subsystem_(ArduinoSubsystem::getInstance())
{
	temperature_ = 0.0;
	humidity_ = 0.0;
	pressure_ = 0.0;
	updateflg_ = 0;
}

/**
 * @brief 環境センサー値取得(更新)処理.
 */
int BMEControl::get(bool async)
{
	updateflg_ = 0;
	if(async){
		bmeAsync_ = std::async(std::launch::async, BMEControl_getImpl_);
		return 0;
	}else{
		return BMEControl_getImpl_();
	}
}

/**
 * @brief 環境センサー補正値設定処理.
 */
int BMEControl::setAdjust(bool async, uint8_t tmpCoef_, char tmp_, uint8_t humCoef_, char hum_, uint8_t pressCoef_, char press_, bool eep_)
{
	if(async){
		bmeAsync_ = std::async(std::launch::async, BMEControl_setAdjImpl_, tmpCoef_, tmp_, humCoef_, hum_, pressCoef_, press_, eep_);
		return 0;
	}else{
		return BMEControl_setAdjImpl_(tmpCoef_, tmp_, humCoef_, hum_, pressCoef_, press_, eep_);
	}
}


}



