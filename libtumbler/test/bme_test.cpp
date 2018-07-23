/*
 * @file bme_test.cpp
 * @brief BMEControl の連続受信単体試験
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/22 
 */

#include <iostream>
#include <unistd.h>
#include "tumbler/tumbler.h"
#include "tumbler/bme.h"


int main(int argc, char** argv)
{
	using namespace tumbler;
	{
		// 環境センサーの計測値を取得。
		BMEControl& bme = BMEControl::getInstance();
		
		// 補正値設定。EEPROMに保存しない.
		bme.setAdjust(false, 100, 0, 100, 0, 100, 0, false);
		sleep(1);
		
		for(int i = 0; i < 10; ++i){
			bme.get(false);
			sleep(1);
			std::cout << "Temp=" << bme.getTemp() << " Hum=" << bme.getHum() << " Press=" << bme.getPress() << std::endl;
		}
	}
	return 0;
}


