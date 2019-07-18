/*
 * @file buttons5.cpp
 * \~english
 * @brief 
 * \~japanese
 * @brief タッチボタンの利用例５、１〜４とは異なる方式で、短押し、長押しを検出します。合わせて複数ボタン押し検出機能も追加しています。また長押し検出機能においては、一定時間を長押しであるとして、それが繰り返された場合、
 * 連続判定されるような仕様としています。
 * @details １〜４では、単純なワンショットタイマーによって、長押しであるか短押しであるかを検出していました。しかしこの方式では、指が一瞬だけ触れたり離れたりすることによって、意図しない動作につながる場合がありました。
 * このため本サンプルでは、短押しについては、連続三回 push 判定がなされない限りは短押しとはしない（連続 2 回までの場合は無視する）という実装とし、一瞬指が触れた、というような状況での誤検出を抑制しています。
 * 長押しについては、連続で指定回数以上 push 判定が為された場合に長押しであると判定することとしています。
 * また、１〜４では、１つのボタンについて短押し、長押しをそれぞれ判定することができましたが、同時に押された場合、それぞれ別々に判定され、別々にオーバーライド関数が呼び出されていましたが、
 * このサンプルでは、複数ボタンが同時に押された場合、一定の基準で同時性を判定し、オーバーライド関数に複数ボタンの ID を与えて合わせて呼び出すようにするこで、同時に押されたか否かを検出する機能が追加されています。
 * @attention 同時押しが検出できていますが、Tumbler T-01 トップパネルの４つのボタンは近接しており、複数ボタンに誤って触れてしまう場合があることに留意してください。このため、複数ボタンの同時検出を UX に組み込むことは
 * 積極的には推奨されません。
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
 * @class MultiButtonEvent
 * @brief
 */
class MultiButtonEvent
{
public:
	/**
	 * @brief コンストラクタ
	 * @param [in] longPushTime 長押判定時間。詳細はクラスの説明を参照してください。
	 */
	MultiButtonEvent(int longPushCount) : longPushCount_(longPushCount){}

	virtual ~MultiButtonEvent(){}

	void Action(int buttonId, ButtonState state)
	{
		if(state == ButtonState::pushed_){
			buttonsData_[buttonId].counter_push_.store(buttonsData_[buttonId].counter_push_.load()+1);
			if(buttonsData_[buttonId].counter_push_.load() > longPushCount_){ // 長押し
				std::vector<int> pressedButtonIds; // 全ての押されているボタンを確認
				for(int i=0;i<4;++i){
					if(buttonsData_[i].counter_push_ > longPushCount_ / 2){ // 長押し時間の半分以上満たす必要があるとする
						pressedButtonIds.push_back(i);
						buttonsData_[i].onLongPushCalled_.store(true);
					}else{
						buttonsData_[i].onLongPushCalled_.store(false);
					}
					buttonsData_[i].counter_push_.store(0); // リセット（連続判定を許容する）
				}
				buttonsData_[buttonId].action_ = std::async(std::launch::async, &MultiButtonEvent::OnLongPush, this, pressedButtonIds);
			}
		}else if(state == ButtonState::released_){
			if(buttonsData_[buttonId].counter_push_.load() > 3 && buttonsData_[buttonId].onLongPushCalled_.load() == false){ // 規定回数未満の push->release は無視し、長押し判定が１回でも為された直後は無視
				// 他のボタンがリリース判定中であるかどうかを判断
				bool release_checking = false;
				for(int i=0;i<4;++i){
					if(buttonsData_[i].releaseChecking_.load() == true){
						release_checking = true;
						break;
					}
				}
				buttonsData_[buttonId].releaseChecking_.store(true);
				if(!release_checking){
					// 他のボタンがリリース判定中でない場合のみ、このボタンでリリース判定に入る
					buttonsData_[buttonId].oneshotTimer_ = std::async(std::launch::async, [this,buttonId]{
							std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 同時リリース判定時間 500 ms とした
							std::vector<int> releasedButtonIds; // 全てのボタンを確認
							for(int i=0;i<4;++i){
								if(buttonsData_[i].releaseChecking_.load() == true){
									releasedButtonIds.push_back(i);
									buttonsData_[i].releaseChecking_.store(false);
								}
							}
							buttonsData_[buttonId].action_ = std::async(std::launch::async, &MultiButtonEvent::OnPush, this, releasedButtonIds);
					});
				}
			}
			buttonsData_[buttonId].onLongPushCalled_.store(false);
			buttonsData_[buttonId].counter_push_.store(0);
		}
	}

protected:
	/**
	 * @brief ボタンが短押された場合に呼び出される関数
	 * @param [in] buttonId ボタンID[0,3]
	 */
	virtual void OnPush(const std::vector<int>& buttonIds) = 0;

	/**
	 * @brief ボタンが長押しされた場合に呼び出される関数
	 * @param [in] buttonId ボタンID[0,3]
	 */
	virtual void OnLongPush(const std::vector<int>& buttonIds) = 0;

private:
	class ButtonData
	{
	public:
		ButtonData() : counter_push_(0), onLongPushCalled_(false), releaseChecking_(false) {}
		std::future<void> action_;
		std::future<void> oneshotTimer_;
		std::mutex mutex_;
		std::atomic<int> counter_push_;
		std::atomic<bool> onLongPushCalled_;
		std::atomic<bool> releaseChecking_;
	};

	ButtonData buttonsData_[4];
	int longPushCount_;
};


/**
 * @class MyButtonEvent
 * @brief ボタンイベントに対応したアクションの実装。OnPush() 関数と OnLongPush() 関数は非同期的に呼び出されます。
 * @details このクラスには、ボタンイベントに対応した、ユーザー定義アクションのみを実装します。どのようなタイミングで、どのようなボタンイベントが
 * 発生され、どのように呼び出されるか等の処理については、親クラスに実装されています。
 */
class MyButtonEvent : public MultiButtonEvent
{
public:
	/**
	 * @brief コンストラクタ
	 * @param [in] longPushTime 長押しと判定する時間[ms]
	 */
	MyButtonEvent(int longPushCount) : MultiButtonEvent(longPushCount) {}
protected:

	/**
	 * @brief 短押しの場合に呼び出される関数。この関数は、ボタンが離された直後に 1 回のみ呼び出される。
	 * @details この関数は、親クラスによって、独立したスレッドで実行されます。この関数の実行は親クラスの実装によってキューイングされます。
	 * すなわち、この関数の処理を行っている最中に、新たにボタン操作が発生した場合、当該処理の終了後、逐次的に次回処理が実行されます。
	 * @param [in] buttonId ボタンID[0,3]
	 */
	void OnPush(const std::vector<int>& buttonIds) override
	{
		//
		// ここに短押しの場合の適切なコードを実装します。
		//
		if(buttonIds.size() == 1){
			std::cout << "Button Pushed (Single): #" << buttonIds[0] << std::endl;
		}else{
			std::cout << "Button Pushed (Multiple): ";
			for(size_t i=0;i<buttonIds.size();++i){
				std::cout << "#" << buttonIds[i] << " ";
			}
			std::cout << std::endl;
		}
	}

	/**
	 * @brief 長押しの場合に呼び出される関数。この関数は、長押し判定時間が経過した直後に 1 回のみ呼び出される。
	 * @details この関数は、親クラスによって、独立したスレッドで実行されます。この関数の実行は親クラスの実装によってキューイングされます。
	 * すなわち、この関数の処理を行っている最中に、新たにボタン操作が発生した場合、当該処理の終了後、逐次的に次回処理が実行されます。
	 * @param [in] buttonId ボタンID[0,3]
	 * @note OnPush() 関数とは異なり、長押し後、ボタンが離された際には何らも呼び出されないことに留意してください。
	 */
	void OnLongPush(const std::vector<int>& buttonIds) override
	{
		//
		// ここに長押しの場合の適切なコードを実装します。
		//
		if(buttonIds.size() == 1){
			std::cout << "Button Long Pushed (Single): #" << buttonIds[0] << std::endl;
		}else{
			std::cout << "Button Long Pushed (Multiple): ";
			for(size_t i=0;i<buttonIds.size();++i){
				std::cout << "#" << buttonIds[i] << " ";
			}
			std::cout << std::endl;
		}
	}
};

/**
 * @brief タッチボタンが押された、もしくはリリースされた際に呼ばれるコールバック関数
 * @details 生計測値は syslog に出力していますので、`tail -f /var/log/syslog` 等で観察することができます。
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
	MyButtonEvent buttonEvent(10); // 10 回連続判定以上を長押しとします

	// タッチボタンを開始します
	Buttons& buttons = Buttons::getInstance(ButtonStateFunc, static_cast<void*>(&buttonEvent));
	buttons.start();
	std::cout << "ボタンにタッチするとコールバック関数が呼ばれます。このプログラムは 30 秒で終了します..." << std::endl;
	sleep(60); // 60 秒待ちます
	buttons.stop();
	return 0;
}



