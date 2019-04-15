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
#include <cmath>
#include <syslog.h>

//#define SENSOR_VALUE_OUTPUT_DEBUG

namespace tumbler{

int monitorAsync_(ButtonStateCallback func, void* userdata, std::atomic<bool>* stopflag)
{
	int errorno = 0;
	ButtonInfo binfo;
	uint8_t buttonValue[4];
	std::vector<ButtonState> prevState(4);
	std::vector<ButtonState> currState(4);
	for(int i=0;i<4;++i){
		prevState[i] = ButtonState::none_;
	}
	bool first_process = true; // 初回のセンシング処理例外のためのフラグ
	int baseline = 0; // 4 ボタン平均の移動平均によるベースライン追跡簡易処理とする
	int localCounterFromLEDRingChange = 0; // LED リング変化後にボタンステートへの影響が出るまでの遅延をカバーする

	while(stopflag->load() == false){
		// 通信（グローバルサブシステムロック）
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		{
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

		// ベースライン補正処理
		bool all_none_state = true;
		for(int i=0;i<4;++i){
			if(prevState[i] != ButtonState::none_){
				all_none_state = false;
				break;
			}
		}
		if(all_none_state || subsystem.c_status_ledringChange_.load()){ // 全ボタンが none_ ステートもしくは LED リングが変化したとき
			// ベースライン追跡処理を行う
			float mean = 0;
			for(int i=0;i<4;++i){
				unsigned short p = static_cast<unsigned short>(buttonValue[i]);
				mean += static_cast<float>(p);
			}
			mean = mean / 4.0F;

			float new_baseline_candidate = static_cast<float>(baseline) * 0.5F + mean * 0.5F;
			if(first_process || subsystem.c_status_ledringChange_.load()){ // 初回例外もしくは LED リングが変化したときは、急変動制約を外す
				baseline = static_cast<int>(new_baseline_candidate); // 初回はベースラインを更新し
				first_process = false;
			}else{
				// ベースラインは移動平均による簡易処理だが、1 ステップで、前回のベースラインからしきい値以上の急変動を採用しない
				if(std::abs(new_baseline_candidate - static_cast<float>(baseline)) < static_cast<float>(baseline) * 0.5){
					// baseline の変動が 50% 以下である
					baseline = static_cast<int>(new_baseline_candidate);
				}
			}
		}

		if(subsystem.c_status_ledringChange_.load()){
			// 判定を行わない
#ifdef SENSOR_VALUE_OUTPUT_DEBUG
			std::cout << "baseline = " << baseline << " [ LED STATUS CHANGE ]" << std::endl;
#endif
			localCounterFromLEDRingChange++;
			if(3 < localCounterFromLEDRingChange){ // 3 計測分は無判定区間とする
				subsystem.c_status_ledringChange_.store(false); // 消費したので false を記録
				localCounterFromLEDRingChange = 0;
			}
		}else{
			// 判定を行う
#ifdef SENSOR_VALUE_OUTPUT_DEBUG
			std::cout << "baseline = " << baseline << std::endl;
#endif
			for(int i=0;i<4;++i){
				unsigned short p = static_cast<unsigned short>(buttonValue[i]);
				int corrected_p = static_cast<int>(p) - baseline; // ベースラインをサブトラクション（ここで負の値になることもある）
#ifdef SENSOR_VALUE_OUTPUT_DEBUG
				std::cout << "#" << i << " corrected_p = " << corrected_p << std::endl;
#endif
				if(30 < corrected_p){
					currState[i] = ButtonState::pushed_;
				}else{
					currState[i] = ButtonState::none_;
				}
			}
		}

		// コールバックを呼ぶかどうかの決定
		// 全ボタンが none_ だったときは呼ばない、ただし、前回がそうではないときのみ 1 回だけ呼ぶ（released_ ステート）
		bool call_callback = false;

		bool prev_has_pushed_state = false; // 前回に pushed_ ステートがあるかどうか
		for(int i=0;i<4;++i){
			if(prevState[i] == ButtonState::pushed_){
				prev_has_pushed_state = true; // 前回に pushed_ ステートがあった
				break;
			}
		}
		if(prev_has_pushed_state){
			// 前回 pushed_ ステートがあったときは、今回がどのようなステートでもコールバック関数を呼ぶ必要があり、今回が
			// none_ ステート（すなわち pushed_ -> none_ 変化）だったときは none_ を release_ ステートと変換する
			call_callback = true;
			// pushed_->none_ 変化が存在する場合は、released_ ステートへ変換する
			for(int i=0;i<4;++i){
				if(prevState[i] == ButtonState::pushed_ && currState[i] == ButtonState::none_){
					currState[i] = ButtonState::released_;
				}
			}
		}else{
			// 前回 pushed_ ステートがなかったとき、すなわち全て none_（もしくは release_）ステートだったときは、今回が
			// pushed_ ステートを含まない限り、コールバック関数を呼ぶ必要はない
			for(int i=0;i<4;++i){
				if(currState[i] == ButtonState::pushed_){
					call_callback = true; // コールバック関数を呼ばなければならない
					break;
				}
			}
		}

		if(call_callback){
			// コールバック関数を呼ぶ
			binfo.baselines_.clear();
			binfo.corrValues_.clear();
			for(int i=0;i<4;++i){
				unsigned short p = static_cast<unsigned short>(buttonValue[i]);
				int corrected_p = static_cast<int>(p) - baseline; // ベースラインをサブトラクション（ここで負の値になることもある）
				binfo.baselines_.push_back(baseline);
				binfo.corrValues_.push_back(corrected_p);
			}
			func(currState, binfo, userdata);
		}

		// 過去ステートを記録する
		for(int i=0;i<4;++i){
			prevState[i] = currState[i];
		}

		// 仮に 1/100 秒のウェイトを置くが、このウェイトはループスピードを律速しない
		usleep(1000*100*1);
	}
	return errorno;
}

Buttons& Buttons::getInstance(ButtonStateCallback func, void* userdata)
{
	static Buttons instance(func, userdata);
	return instance;
}

Buttons::Buttons(ButtonStateCallback func, void* userdata) :
		subsystem_(ArduinoSubsystem::getInstance()),
		callback_(func),
		status_(false),
		stopflag_(false),
		userdata_(userdata)
{}

void Buttons::start()
{
	status_ = true;
	monitor_ = std::async(std::launch::async, monitorAsync_, callback_, userdata_, &stopflag_);
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


