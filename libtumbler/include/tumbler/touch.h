/*
 * @file touch.h
 * @brief タッチセンサーの制御クラス
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_TOUCH_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_TOUCH_H_

#include "tumbler/tumbler.h"

#include <memory>
#include <future>
#include <vector>

namespace tumbler{

#define TOUCH_THRESH_MAX (20)
#define TOUCH_THRESH_MIN (100)
#define GetTouchThresh(x)	(TOUCH_THRESH_MIN + x * (TOUCH_THRESH_MAX - TOUCH_THRESH_MIN))
	
#define TOUCH_STATE_OFF   (0)
#define TOUCH_STATE_SHORT (2)
#define TOUCH_STATE_LONG  (4)
	
/**
 * @class TouchSensor
 * @brief タッチセンサーを保持するシングルトンクラス。
 */
class DLL_PUBLIC TouchSensor
{
public:
	static TouchSensor& getInstance();

	/**
	 * @brief シリアル受信した内容でデータを更新する.
	 */
	int update(char* data, int datasize);
	int isUpdate(){return updateFlg_;}

	/**
	 * @brief タッチセンサーの状態を取得する
	 */
	int readState(bool async);

	int getStateNow(int target);
	int getStateVal(int target);
	int* getStatePtr();
	
	/**
	 * @brief タッチセンサーの感度を取得する
	 */
	int readThresh(bool async);
	
	int getThreshVal(int target);
	int* getThreshPtr();

	/**
	 * @brief タッチセンサーの計測値を取得する
	 */
	int readCharge(bool async);
	
	int getChargeVal(int target);
	int* getChargePtr();

	/**
	 * @brief タッチセンサーの感度を設定する
	 */
	int setThreshVal(bool async, uint8_t btn1, uint8_t btn2, uint8_t btn3, uint8_t btn4, bool eep);
	int setThreshPer(bool async, float btn1, float btn2, float btn3, float btn4, bool eep);
	
	/**
	 * @brief タッチセンサーの常時レポート機能のONOFFを切り替える
	 */
	int setReport(bool async, uint8_t onoff);
  
	/**
	 * @brief タッチセンサーのコールバック関数を登録する
	 */
	int setCallBackFnc(void (*callbackFnc)(char*,int));
	
	int stateVal_[4];  // ONOFF状態.
	int threshVal_[4]; // 感度(閾値)
	int chargeVal_[4]; // センサー値.
	
private:

	TouchSensor();
	TouchSensor(const TouchSensor&);
	TouchSensor &operator=(const TouchSensor&);
	
	
	
	ArduinoSubsystem& subsystem_;
	std::future<int> touchAsync_;
	int updateFlg_;
	
};

}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_LEDRING_H_ */
