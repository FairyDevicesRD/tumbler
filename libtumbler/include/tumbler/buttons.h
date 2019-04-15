/*
 * @file buttons.h
 * \~english
 * @brief 
 * \~japanese
 * @brief タッチボタン制御クラス
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
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_BUTTONS_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_BUTTONS_H_

#include "tumbler/tumbler.h"

#include <memory>
#include <future>
#include <vector>

namespace tumbler
{

/**
 * @class ButtonState
 * @brief 各タッチボタンのセンシング状態
 */
enum class DLL_PUBLIC ButtonState
{
	none_,    //!< 何も検知されていない状態
	pushed_,  //!< 指の接触が検知された状態
	released_,//!< 接触検知状態から、指が離された状態（pushed_ ステートからの変化として 1 回のみ呼ばれ、以降は none_ ステートになる）
};

/**
 * @class ButtonInfo
 * @brief 各タッチボタンのセンシング状態の詳細
 * @note ユーザーアプリケーションで利用することはほぼありません
 */
class DLL_PUBLIC ButtonInfo
{
public:
	std::vector<int> baselines_;  //!< 各タッチボタンのベースライン補正値（非接触状態の測定値）
	std::vector<int> corrValues_; //!< ベースライン補正値を減算した各タッチボタンの補正済計測値
};

using ButtonStateCallback = void (*)(std::vector<ButtonState>, ButtonInfo, void*);

class DLL_PUBLIC Buttons
{
public:
	static Buttons& getInstance(ButtonStateCallback func, void* userdata);

	void start();
	void stop();

private:
	Buttons(ButtonStateCallback, void*);
	Buttons(const Buttons&);
	Buttons &operator=(const Buttons&);
	ArduinoSubsystem& subsystem_;
	ButtonStateCallback callback_;
	std::future<int> monitor_;
	std::atomic<bool> stopflag_;
	bool status_;
	void* userdata_;
};

}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_BUTTONS_H_ */
