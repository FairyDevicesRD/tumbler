/*
 * @file irio.h
 * \~english
 * @brief Control IR I/O
 * \~japanese
 * @brief 赤外線 I/O の制御
 * \~ 
 * @author Masato Fujino, created on: May 16, 2019
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
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_IRIO_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_IRIO_H_

#include "tumbler/tumbler.h"
#include <atomic>

namespace tumbler{

/**
 * @brief 赤外線信号を外部から受けた際に呼ばれるコールバック関数
 * @param [in] signalHash 受信信号を一意に識別するためのハッシュ値。異なる信号に対して異なるハッシュ値が与えられます。
 * これをキーとして、ユーザープログラムにおいて動作を変えることができます。
 * @param [in] tick 受信した時刻値（マイクロ秒を表し、2^32 ms（約 1.2 h）で周回する）
 * @param [in] userdata 任意のユーザーデータ
 * @note tick 値同士の差は常に想定通りの値を返すため、周回について配慮する必要はありません。
 */
using IRSignalReceiptCallback = void (*)(uint32_t signalHash, uint32_t tick, void* userdata);

/**
 * @brief 自発光の赤外線反射を検出した際に呼ばれるコールバック関数
 * @details 近接センサーとして利用することができる
 * @param [in] tick 受信した時刻値（マイクロ秒を表し、2^32 ms（約 1.2 h）で周回する）
 * @param [in] userdata 任意のユーザーデータ
 * @note tick 値同士の差は常に想定通りの値を返すため、周回について配慮する必要はありません。
 */
using IRProximityDetectionCallback = void (*)(uint32_t tick, void* userdata);

/**
 * @class IRIO
 * @brief 赤外線 I/O の管理クラス、ライブラリの利用者がこのクラスを直接利用することはありません。
 */
class DLL_PUBLIC IRIO
{
public:
	static IRIO& getInstance(){
		static IRIO instance;
		return instance;
	}
	int startReceiver(IRSignalReceiptCallback func_, void* userdata);
	int startReceiver(IRProximityDetectionCallback func_, void* userdata);
	int stopReceiver();

	bool signalReceipt() const { return flagSignalReceipt_.load(); }
	bool proximityDetection() const { return flagProximityDetection_.load(); }
	void signalReceipt(bool f) { return flagSignalReceipt_.store(f); }
	void proximityDetection(bool f) { return flagProximityDetection_.store(f); }
	void commonGPIOCallback(int gpio, int level, uint32_t tick);

private:
	IRIO() : in_code_(0), hash_(2166136261U), edges_(1), t1_(0), t2_(0), t3_(0), t4_(0){
		flagSignalReceipt_.store(false);
		flagProximityDetection_.store(false);
	}
	IRIO(const IRIO&);
	IRIO &operator=(const IRIO&);
	void hash(int val1, int val2);
	int startReceiverC();

	std::atomic<bool> flagSignalReceipt_;
	IRSignalReceiptCallback funcSignalReceipt_ = nullptr;
	void *userdataSignalReceipt_ = nullptr;

	std::atomic<bool> flagProximityDetection_;
	IRProximityDetectionCallback funcProximityDetection_ = nullptr;
	void *userdataProximityDetection_ = nullptr;

	int in_code_;
	uint32_t hash_;
	int edges_;
	uint32_t t1_,t2_,t3_,t4_;
	const int receivergpio_ = 4;
	std::mutex sslock_;
};

/**
 * @class IRPRoximitySensor
 * @brief 赤外線 I/O を利用した近接センサー
 */
class DLL_PUBLIC IRProximitySensor
{
public:
	static IRProximitySensor& getInstance(IRProximityDetectionCallback func, void* userdata);
	/**
	 * @class Sensitivity
	 * @brief 近接センサーの感度
	 */
	enum class Sensitivity
	{
		high_,   //!< 高感度（概ね 1 m 以上程度で反応）
		medium_, //!< 中感度（概ね 60cm 程度で反応）
		low_,    //!< 低感度（概ね 30cm 程度で反応）
	};
	/**
	 * @brief 近接センサーを開始する。センサー感度はデフォルトで高感度に設定されています。
	 * @details Tumbler は近接センサーとして利用する赤外線 LED の発振を開始します。この様子は、スマートフォンのカメラ等を
	 * Tumbler 上部の LED リング正面に向けることで目視することができます。
	 * @return 0 if success
	 */
	int start();
	/**
	 * @brief 指定したセンサー感度で近接センサーを開始する
	 * @param [in] sensitivity センサー感度
	 * @return 0 if success
	 */
	int start(IRProximitySensor::Sensitivity sensitivity);
	/**
	 * @brief 近接センサーを終了する
	 * @return 0 if success
	 */
	int stop();
private:
	IRProximitySensor(IRProximityDetectionCallback func, void* userdata);
	IRProximitySensor(const IRProximitySensor&);
	IRProximitySensor &operator=(const IRProximitySensor&);

	IRProximityDetectionCallback func_;
	void* userdata_;
	IRIO& irio_;
	IRProximitySensor::Sensitivity sensitivity_;
};

/**
 * @class IRSignalReceiver
 * @brief 外部からの赤外線信号を受信するためのクラス。一般の赤外線リモコン等を利用することが出来ます。
 */
class DLL_PUBLIC IRSignalReceiver
{
public:
	static IRSignalReceiver& getInstance(IRSignalReceiptCallback func, void* userdata);
	/**
	 * @brief 赤外線信号受信を開始します
	 * @return 0 if success
	 */
	int start();
	/**
	 * @brief 赤外線信号受信を終了します
	 * @return 0 if success
	 */
	int stop();
private:
	IRSignalReceiver(IRSignalReceiptCallback func, void* userdata);
	IRSignalReceiver(const IRSignalReceiver&);
	IRSignalReceiver &operator=(const IRSignalReceiver&);

	IRSignalReceiptCallback func_;
	void* userdata_ ;
	IRIO& irio_;
};

}



#endif /* LIBTUMBLER_INCLUDE_TUMBLER_IRIO_H_ */
