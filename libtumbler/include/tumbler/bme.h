/*
 * @file bme.h
 * @brief BME 環境センサーの制御クラス
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_BME_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_BME_H_

#include "tumbler/tumbler.h"

#include <memory>
#include <future>
#include <vector>

namespace tumbler{


/**
 * @class BMEControl
 * @brief BME 環境センサーを保持するシングルトンクラス。
 */
class DLL_PUBLIC BMEControl
{
public:
	static BMEControl& getInstance();

	/**
	 * @brief 環境センサーの計測値を取得する.
	 */
	int get(bool async);

	/**
	 * @brief 環境センサーの補正値を設定する.
	 */
	int setAdjust(bool async, uint8_t tmpCoef_, char tmp_, uint8_t humCoef_, char hum_, uint8_t pressCoef_, char press_, bool eep_);
	
	/**
	 * @brief 環境センサーの計測結果を更新する.
	 */
	void update(char* data, int dataSize);

	/**
	 * @brief 更新が完了したかチェック.
	 */
	int isUpdate(){return updateflg_;}
	
	/**
	 * @brief 温度情報の入出力処理.
	 */
	float getTemp(){return temperature_;}
	void setTemp(float tmp){temperature_ = tmp;}

	/**
	 * @brief 湿度情報の入出力処理.
	 */
	float getHum(){return humidity_;}
	void setHum(float tmp){humidity_ = tmp;}

	/**
	 * @brief 気圧情報の入出力処理.
	 */
	float getPress(){return pressure_;}
	void setPress(float tmp){pressure_ = tmp;}
	
private:

	BMEControl();
	BMEControl(const BMEControl&);
	BMEControl &operator=(const BMEControl&);
	ArduinoSubsystem& subsystem_;
	std::future<int> bmeAsync_;

	float temperature_;	// 温度
	float humidity_;	// 湿度
	float pressure_;	// 気圧

	float adj_temperature_;	// 温度補正値.
	float adj_humidity_;	// 湿度補正値.
	float adj_pressure_;	// 気圧補正値.
	
	int updateflg_;
};

}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_BME_H_ */
