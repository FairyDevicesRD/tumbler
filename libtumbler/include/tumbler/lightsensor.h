/*
 * @file lightsensor.hpp
 * \~english
 * @brief Get measured value from light sensor (LTR-329ALS) which includes visible and IR light power.
 * \~japanese
 * @brief 光センサー（LTR-329ALS）からの計測値の取得
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
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_LIGHTSENSOR_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_LIGHTSENSOR_H_

#include "tumbler/tumbler.h"

namespace tumbler{

/**
 * @class LightSensor
 * @brief 光センサー（LTR-329ALS）を表すクラス、シングルトン・インスタンスとして利用します。
 */
class DLL_PUBLIC LightSensor
{
public:
	/**
	 * @brief 光センサーのシングルトン・インスタンスを取得します。
	 * @return 光センサーのシングルトン・インスタンス
	 */
	static LightSensor& getInstance();

	/**
	 * @brief 可視光の明るさを取得します
	 * @return 可視光領域の明るさ[lux]
	 */
	unsigned int light();

private:
	LightSensor();
	LightSensor(const LightSensor&);
	LightSensor &operator=(const LightSensor&);
};

}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_LIGHTSENSOR_H_ */
