/*
 * @file envsensor.cpp
 * \~english
 * @brief 
 * \~japanese
 * @brief 
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

#include "tumbler/envsensor.h"
#include "thirdparty/raspberry-pi-bme280/bme280.h"
#include <stdexcept>
#include <wiringPiI2C.h>
#include <unistd.h>
#include <iostream>
#include <cmath>

namespace tumbler{

class EnvSensor::BME280S
{
public:
	BME280S(){
		fd_ = wiringPiI2CSetup(BME280_ADDRESS);
		if(fd_ < 0){
			throw std::runtime_error("Environmental sensor could not be detected. I2C is disabled or BME280 is not installed.");
		}
		 readCalibrationData(fd_, &cal_);
		 wiringPiI2CWriteReg8(fd_, 0xf2, 0x01);   // humidity oversampling x 1
		 wiringPiI2CWriteReg8(fd_, 0xf4, 0x25);   // pressure and temperature oversampling x 1, normal mode.
	}
	~BME280S()
	{
		::close(fd_);
	}

	int fd_;
	bme280_calib_data cal_;
};

EnvSensor::EnvSensor() :
		temperatureCalibrationValue_(0),
		bme280s_(new BME280S)
{
	float dummy = temperature();
	(void) dummy;
}

EnvSensor& EnvSensor::getInstance()
{
	static EnvSensor instance;
	return instance;
}

float EnvSensor::temperature()
{
	bme280_raw_data raw;
	getRawData(bme280s_->fd_, &raw);
	int32_t t_fine = getTemperatureCalibration(&bme280s_->cal_, raw.temperature);
	int32_t rc = static_cast<int32_t>(13 + temperatureCalibrationValue_);
	int32_t t_e_fine = ((rc << 8) * 100 - 128) / 5;
	int32_t t = t_fine - t_e_fine;
	return compensateTemperature(t);
}

float EnvSensor::humidity()
{
	bme280_raw_data raw;
	getRawData(bme280s_->fd_, &raw);
	int32_t t_fine = getTemperatureCalibration(&bme280s_->cal_, raw.temperature);
	float p_t = compensateTemperature(t_fine); // 校正前の温度
 	int32_t rc = static_cast<int32_t>(13 + temperatureCalibrationValue_);
	int32_t t_e_fine = ((rc << 8) * 100 - 128) / 5;
	int32_t t = t_fine - t_e_fine;
	float q_t = compensateTemperature(t); // 校正後の温度
	float p_h = compensateHumidity(raw.humidity, &bme280s_->cal_, t); // 校正後温度での補償値
	float p_et = 6.1078F*std::pow(10, (7.5F*p_t) / (p_t+237.3F)); // 計測値での飽和水蒸気圧[hPa]
	float p_at = 217.0F*p_et / (p_t+273.15); // 計測値での飽和水蒸気量[g/m3]
	float p_a = p_at * p_h / 100.0F; // 水蒸気量
	float q_et = 6.1078F*std::pow(10, (7.5F*q_t) / (q_t+237.3F)); // 校正後温度での飽和水蒸気圧[hPa]
	float q_at = 217.0F*q_et / (q_t+273.15); // 校正後温度での飽和水蒸気量[g/m3]
	float q_h = p_a / q_at * 100.0F; // 校正後温度換算での相対湿度[%]
	return q_h;
}

float EnvSensor::pressure()
{
	bme280_raw_data raw;
	getRawData(bme280s_->fd_, &raw);
	int32_t t_fine = getTemperatureCalibration(&bme280s_->cal_, raw.temperature);
	return compensatePressure(raw.pressure, &bme280s_->cal_, t_fine) / 100;
}

}
