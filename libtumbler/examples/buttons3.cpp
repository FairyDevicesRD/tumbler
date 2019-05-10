/*
 * @file buttons.cpp
 * \~english
 * @brief Example program of using touch buttons
 * \~japanese
 * @brief タッチボタンの利用例（マルチタッチ無効）
 * \~ 
 * @author Masato Fujino, created on: Mar 30, 2019
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
#include <memory>
#include <future>
#include <vector>
#include <string>
#include <unistd.h>

using namespace tumbler;

/**
 * @brief タッチボタンが押された、もしくはリリースされた際に呼ばれるコールバック関数
 * @details コールバック関数内で時間のかかる処理を行うと、libtumbler によるタッチセンサーのセンシング間隔が大きくなり、タッチした際の反応速度が遅くなるなどの悪影響があります。
 * コールバック関数内ではデータのコピーのみを行うなど、軽量な実装を行うようにしてください。
 * @param [in] state ４つのタッチボタンのそれぞれの状態
 * @param [in] info ４つのタッチボタンのタッチ計測値（通常は使いませんが、このサンプルでは内容を示すために表示しています）
 * @param [in] userdata 任意のユーザーデータ
 */
void ButtonStateFunc(std::vector<ButtonState> state, ButtonInfo info, void* userdata)
{
	// 通常の短押しの検出例
	std::cout << "callback called" << std::endl;
	for(int i=0;i<4;++i){
		if(state[i] == ButtonState::pushed_){
			std::cout << "PUSHED (" << info.corrValues_[i] << ", baseline=" << info.baselines_[i] << ")" << std::endl;
		}else if(state[i] == ButtonState::released_){
			std::cout << "RELEASED (" << info.corrValues_[i] << ")" << std::endl;
		}else if(state[i] == ButtonState::none_){
			std::cout << info.corrValues_[i] << std::endl;
		}
	}
}

int main(int argc, char** argv)
{
	// LED リングを消灯しておきます（本サンプルプログラムには不要です）
	LEDRing& ring = LEDRing::getInstance();
	ring.reset(false);

	// タッチボタンを開始します
	ButtonDetectionConfig config;
	config.multiTouchDetectionEnabled_ = false; // マルチタッチ無効
	Buttons& buttons = Buttons::getInstance(ButtonStateFunc, config, nullptr);
	buttons.start();
	std::cout << "ボタンにタッチするとコールバック関数が呼ばれます。このプログラムは 30 秒で終了します..." << std::endl;
	sleep(30); // 30 秒待ちます
	buttons.stop();
	return 0;
}

