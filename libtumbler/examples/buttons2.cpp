/*
 * @file buttons2.cpp
 * \~english
 * @brief 
 * \~japanese
 * @brief タッチボタンの利用例２、より実際的な例。長押しの判定機能がある。
 * \~ 
 * @author Masato Fujino, created on: Apr 16, 2019
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

#include <tumbler/buttons.h>
#include <tumbler/ledring.h>
#include <iostream>
#include <future>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <syslog.h>
#include <unistd.h>

using namespace tumbler;

/**
 * @class ButtonEvent
 * @brief ボタンイベントをハンドルするための基底クラスの一例、純粋仮想関数である OnPush(), OnLongPush() メンバー関数をオーバーライドして利用します。
 * 長押しであるかどうかの判定時間をコンストラクタで指定することができます。
 * @details 長押判定時間以上、ボタンが継続的に押された場合には、長押判定時間経過直後に OnLongPush() 関数が 1 回のみ呼び出されます。
 * 長押判定時間以下、ボタンが継続的に押された場合には、ボタンが離された直後に OnPush() 関数が 1 回のみ呼び出されます。
 * @note 基底クラスでは、ボタンイベントをどのように呼び出すか、派生クラスではボタンイベントに対応したアクションの実装を行っています。イベントの呼び出され方と
 * アクションの内容は独立であるため、メンテナンス性の観点からクラスを分けています。
 */
class ButtonEvent
{
public:
	/**
	 * @brief コンストラクタ
	 * @param [in] longPushTime 長押判定時間。詳細はクラスの説明を参照してください。
	 */
	ButtonEvent(std::chrono::milliseconds longPushTime) : longPushTime_(longPushTime){}

	virtual ~ButtonEvent(){}
	void Action(int buttonId, ButtonState state)
	{
		if(state == ButtonState::pushed_){
			if(buttonsData_[buttonId].status_.load() == 0){ // 初回の push 検出
				buttonsData_[buttonId].status_.store(1);
				buttonsData_[buttonId].oneshotTimer_ = std::async(std::launch::async, [&]{
					std::unique_lock<std::mutex> lock(buttonsData_[buttonId].mutex_);
					if(!buttonsData_[buttonId].condition_.wait_for(lock, longPushTime_, [&]{return !(buttonsData_[buttonId].status_.load() == 1);})){
						// 長押判定時間経過後にまだボタンが押されている場合
						buttonsData_[buttonId].action_ = std::async(std::launch::async, &ButtonEvent::OnLongPush, this, buttonId);
						buttonsData_[buttonId].status_.store(3);
					}else{
						// 長押判定時間経過前にボタンが離された場合
						buttonsData_[buttonId].status_.store(0); // 終了
					}
				});
			} // 連続する 2 回目以降の push 検出については無視して良い
		}else if(state == ButtonState::released_){
			if(buttonsData_[buttonId].status_.load() == 1){
				// 長押判定時間経過前にボタンが離された場合
				buttonsData_[buttonId].action_ = std::async(std::launch::async, &ButtonEvent::OnPush, this,  buttonId);
				buttonsData_[buttonId].status_.store(2);
				buttonsData_[buttonId].condition_.notify_one();
			}else if(buttonsData_[buttonId].status_.load() == 3){
				// 長押判定時間経過後にボタンが離された場合
				buttonsData_[buttonId].status_.store(0); // 終了
			}
		}
	}

protected:
	/**
	 * @brief ボタンが短押された場合に呼び出される関数
	 * @param [in] buttonId ボタンID[0,3]
	 */
	virtual void OnPush(int buttonId) = 0;

	/**
	 * @brief ボタンが長押しされた場合に呼び出される関数
	 * @param [in] buttonId ボタンID[0,3]
	 */
	virtual void OnLongPush(int buttonId) = 0;

private:
	class ButtonData
	{
	public:
		ButtonData() {status_.store(0);}
		std::future<void> action_;
		std::future<void> oneshotTimer_;
		std::mutex mutex_;
		std::condition_variable condition_;
		std::atomic<int> status_;
	};

	ButtonEvent::ButtonData buttonsData_[4];
	std::chrono::milliseconds longPushTime_;
};

/**
 * @class MyButtonEvent
 * @brief ボタンイベントに対応したアクションの実装。OnPush() 関数と OnLongPush() 関数は非同期的に呼び出されます。
 * @details このクラスには、ボタンイベントに対応した、ユーザー定義アクションのみを実装します。どのようなタイミングで、どのようなボタンイベントが
 * 発生され、どのように呼び出されるか等の処理については、親クラスに実装されています。
 */
class MyButtonEvent : public ButtonEvent
{
public:
	/**
	 * @brief コンストラクタ
	 * @param [in] longPushTime 長押しと判定する時間[ms]
	 */
	MyButtonEvent(std::chrono::milliseconds longPushTime) : ButtonEvent(longPushTime) {}
protected:

	/**
	 * @brief 短押しの場合に呼び出される関数。この関数は、ボタンが離された直後に 1 回のみ呼び出される。
	 * @details この関数は、親クラスによって、独立したスレッドで実行されます。この関数の実行は親クラスの実装によってキューイングされます。
	 * すなわち、この関数の処理を行っている最中に、新たにボタン操作が発生した場合、当該処理の終了後、逐次的に次回処理が実行されます。
	 * @param [in] buttonId ボタンID[0,3]
	 */
	void OnPush(int buttonId) override
	{
		//
		// ここに短押しの場合の適切なコードを実装します。
		//
		std::cout << "Button Pushed: #" << buttonId << std::endl;
	}

	/**
	 * @brief 長押しの場合に呼び出される関数。この関数は、長押し判定時間が経過した直後に 1 回のみ呼び出される。
	 * @details この関数は、親クラスによって、独立したスレッドで実行されます。この関数の実行は親クラスの実装によってキューイングされます。
	 * すなわち、この関数の処理を行っている最中に、新たにボタン操作が発生した場合、当該処理の終了後、逐次的に次回処理が実行されます。
	 * @param [in] buttonId ボタンID[0,3]
	 * @note OnPush() 関数とは異なり、長押し後、ボタンが離された際には何らも呼び出されないことに留意してください。
	 */
	void OnLongPush(int buttonId) override
	{
		//
		// ここに長押しの場合の適切なコードを実装します。
		//
		std::cout << "Button Long Pushed: #" << buttonId << std::endl;
	}
};

/**
 * @brief タッチボタンが押された、もしくはリリースされた際に呼ばれるコールバック関数
 * @param [in] state ４つのタッチボタンのそれぞれの状態
 * @param [in] info ４つのタッチボタンのタッチ計測値
 * @param [in] userdata 任意のユーザーデータ
 */
void ButtonStateFunc(std::vector<ButtonState> state, ButtonInfo info, void* userdata){
	syslog(LOG_DEBUG, "Button State Callback Func"); // コールバック関数の生情報をデバッグ用に syslog 出力
	for(int i=0;i<4;++i){
		std::stringstream ss;
		if(state[i] == ButtonState::pushed_){
			ss << "PUSHED (" << info.corrValues_[i] << ", baseline=" << info.baselines_[i] << ")" << std::endl;
		}else if(state[i] == ButtonState::released_){
			ss << "RELEASED (" << info.corrValues_[i] << ")" << std::endl;
		}else{
			ss << info.corrValues_[i] << ", baseline = " << info.baselines_[i] << std::endl;
		}
		syslog(LOG_DEBUG, ss.str().c_str());

		if(state[i] == ButtonState::pushed_ || state[i] == ButtonState::released_){
			reinterpret_cast<MyButtonEvent*>(userdata)->Action(i, state[i]);
		}
	}
}

int main(int argc, char** argv)
{
	// LED リングを消灯しておきます（本サンプルプログラムには不要です）
	LEDRing& ring = LEDRing::getInstance();
	ring.reset(false);

	// ボタンイベントハンドラの準備
	MyButtonEvent buttonEvent(std::chrono::milliseconds(1000)); // 1 秒以上を長押しとします

	// タッチボタンを開始します
	Buttons& buttons = Buttons::getInstance(ButtonStateFunc, static_cast<void*>(&buttonEvent));
	buttons.start();
	std::cout << "ボタンにタッチするとコールバック関数が呼ばれます。このプログラムは 30 秒で終了します..." << std::endl;
	sleep(60); // 60 秒待ちます
	buttons.stop();
	return 0;
}



