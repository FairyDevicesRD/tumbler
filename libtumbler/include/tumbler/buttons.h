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

enum class ButtonState
{
	none_,
	pushed_,
	slided_,
};

using ButtonStateCallback = void (*)(std::vector<ButtonState>);

class DLL_PUBLIC Buttons
{
public:
	static Buttons& getInstance(void (*)(std::vector<ButtonState>));

	void start();
	void stop();

private:
	Buttons(ButtonStateCallback);
	Buttons(const Buttons&);
	Buttons &operator=(const Buttons&);
	ArduinoSubsystem& subsystem_;
	ButtonStateCallback callback_;
	std::future<int> monitor_;
	std::atomic<bool> stopflag_;
	bool status_;
	int baseline_; // 内部補正用
};

}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_BUTTONS_H_ */
