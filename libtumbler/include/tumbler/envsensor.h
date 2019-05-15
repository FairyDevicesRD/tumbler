/*
 * @file envsensor.h
 * \~english
 * @brief Get measured value from environmental sensor which includes temperature, humidity, pressure.
 * \~japanese
 * @brief 環境センサーからの計測値の取得
 * \~ 
 * @author Masato Fujino, created on: May 14, 2019
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
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_ENVSENSOR_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_ENVSENSOR_H_

#include "tumbler/tumbler.h"
#include <memory>

namespace tumbler{

/**
 * @class EnvSensor
 * @brief 環境センサーを表すクラス、シングルトンインスタンスとして利用します。
 */
class DLL_PUBLIC EnvSensor
{
public:
	/**
	 * @brief 環境センサーのシングルトン・インスタンスを取得します。
	 * @return 環境センサーのシングルトン・インスタンス
	 */
	static EnvSensor& getInstance();

	/**
	 * @brief 環境センサーのうち気温センサーのキャリブレーション値をセットします
	 * @params [in] 気温センサーに対するキャリブレーション値[摂氏]
	 * @details 環境センサーのうち、気温センサーは、Tumbler 本体から発生する熱の影響を受け、測定値が上振れする場合があります。気温センサーの計測値が上振れした場合、湿度（相対湿度）センサーの計測値が下振れすることになります。
	 * 熱は特に CPU から発生し、CPU が熱飽和している状況が継続した場合、継続時間に応じて、最大で５度程度、高い値が計測される傾向があります。
	 * デフォルトでは、CPU がアイドル状態の場合に正しい気温が測定されるように調整されていますが、ユーザーアプリケーションに応じる CPU 使用率の平均状況が分かっている場合には、
	 * 第一引数の `temp` を用いて、キャリブレーション値（一般的には正の値）を指定することが有効です。`temp` に指定された
	 * 固定値は、気温センサーの実測値から減算され、気温センサー計測値として `temperature()` 関数の戻り値として返されます。これは、湿度センサーの相対湿度計算にも利用されます。
	 * 気温センサーのキャリブレーションは、別途温度計を用意して行うことができます。ユーザーアプリケーションが起動している状態で５分程度の適当な時間をおき、Tumbler 本体の熱状況を安定させます。
	 * このときの `temperature()` 関数の出力値と、別途用意した温度計の計測値との差分が `temp` に指定すべきキャリブレーション値となります。
	 */
	void calibrateTemperature(float temp) { temperatureCalibrationValue_ = temp; }

	/**
	 * @brief 気温を取得する
	 * @note 測定値は、気温センサーキャリブレーション値の影響を受けます
	 * @return 気温（摂氏）
	 */
	float temperature();

	/**
	 * @brief 相対湿度を取得する
	 * @note 測定値は、コ気温センサーキャリブレーション値の影響を受けます
	 * @return 相対湿度（％）
	 */
	float humidity();

	/**
	 * @brief 気圧を取得する
	 * @return 気圧（hPa）
	 */
	float pressure();

	class DLL_LOCAL BME280S;

private:
	EnvSensor();
	EnvSensor(const EnvSensor&);
	EnvSensor &operator=(const EnvSensor&);

	float temperatureCalibrationValue_;
	std::unique_ptr<BME280S> bme280s_;
};
}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_ENVSENSOR_H_ */
