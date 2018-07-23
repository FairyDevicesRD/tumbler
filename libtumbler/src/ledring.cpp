/*
 * @file ledring.cpp
 * @brief LED リング制御クラスの実装
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */

#include "tumbler/ledring.h"
#include <unistd.h>
#include <mutex>
#include <future>
#include <thread>

namespace tumbler{

const LED operator*(const LED& c, float alpha)
{
	return LED(c.r_*alpha, c.g_*alpha, c.b_*alpha);
}

const LED operator*(float alpha, const LED& c)
{
	return LED(c.r_*alpha, c.g_*alpha, c.b_*alpha);
}

const bool operator==(const LED& c, int v)
{
	return (c.r_ == v && c.g_ == v && c.b_ == v);
}

const bool operator==(int v, const LED& c)
{
	return (c.r_ == v && c.g_ == v && c.b_ == v);
}

const bool operator!=(const LED& c, int v)
{
	return !(c.r_ == v && c.g_ == v && c.b_ == v);
}

const bool operator!=(int v, const LED& c)
{
	return !(c.r_ == v && c.g_ == v && c.b_ == v);
}

Frame::Frame(const LED& background)
{
	for(int i=0;i<k_num_leds_;++i){
		leds_[i] = background;
	}
}

Frame::Frame(const Frame& f)
{
	for(int i=0;i<k_num_leds_;++i){
		leds_[i] = f.leds_[i];
	}
}

uint8_t Frame::toDataForTx(char* data) const
{
	int c = 0;
	for(int i=0;i<k_num_leds_;++i){
		data[c++] = static_cast<uint8_t>(leds_[i].r_);
		data[c++] = static_cast<uint8_t>(leds_[i].g_);
		data[c++] = static_cast<uint8_t>(leds_[i].b_);
	}
	return c;
}

LEDRing& LEDRing::getInstance()
{
	static LEDRing instance;
	return instance;
}

/**
 * @brief デフォルトパターン点灯。
 * @param motion: 動作＝0:停止、1:回転、2:逆回転
 * @param colVar: 点灯位置＝0:1点、1:6分割、2:3分割
 */
static int LEDRing_motionImpl_(uint8_t motion, uint8_t colVar, uint8_t r_, uint8_t g_, uint8_t b_)
{
	{
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("LEDR", 4);
		const uint8_t subtype = 0;
		const uint8_t length  = 5;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.write(reinterpret_cast<const char*>(&motion), 1);
		subsystem.write(reinterpret_cast<const char*>(&colVar), 1);
		subsystem.write(reinterpret_cast<const char*>(&r_), 1);
		subsystem.write(reinterpret_cast<const char*>(&g_), 1);
		subsystem.write(reinterpret_cast<const char*>(&b_), 1);
	}
	
	return 0; // OK
}

/**
 * @brief 消灯。
 */
static int LEDRing_resetImpl_()
{
	{
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("LEDR",4);
		const uint8_t subtype = 1;
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
	}
	
	return 0; // OK
}
	
/**
 * @brief 全点灯。
 */
static int LEDRing_setImpl_(uint8_t r_, uint8_t g_, uint8_t b_)
{
	{
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("LEDR",4);
		const uint8_t subtype = 2;
		const uint8_t length  = 3;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.write(reinterpret_cast<const char*>(&r_), 1);
		subsystem.write(reinterpret_cast<const char*>(&g_), 1);
		subsystem.write(reinterpret_cast<const char*>(&b_), 1);
	}
	
	return 0; // OK
}

/**
 * @brief 1点点灯。
 */
static int LEDRing_setOneImpl_(uint8_t led_, uint8_t r_, uint8_t g_, uint8_t b_)
{
	{
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("LEDR",4);
		const uint8_t subtype = 3;
		const uint8_t length  = 4;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		subsystem.write(reinterpret_cast<const char*>(&led_), 1);
		subsystem.write(reinterpret_cast<const char*>(&r_), 1);
		subsystem.write(reinterpret_cast<const char*>(&g_), 1);
		subsystem.write(reinterpret_cast<const char*>(&b_), 1);
	}
	
	return 0; // OK
}

/**
 * @brief アニメーション(フレーム点灯)。
 */
static int LEDRing_showImpl_(const std::vector<Frame>& frames, int fps)
{
	Timer tm;
	float std_wait_msec = 1000. / static_cast<float>(fps);
	char txdata[128];
	for(size_t i=0;i<frames.size();++i){
		const uint8_t length = frames[i].toDataForTx(txdata);
		{
			ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
			std::lock_guard<std::mutex> lock(subsystem.global_lock_);
			subsystem.write("LEDR", 4);
			const uint8_t subtype = 8;
			subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
			subsystem.write(reinterpret_cast<const char*>(&length), 1);
			tm.start();
			subsystem.write(txdata, static_cast<int>(length));
		}
	  if(fps < 0) break;
		float t1 = tm.stop();
		float mod_wait_msec = std_wait_msec - t1;
		usleep(mod_wait_msec*1000);
	}

	return 0; // OK
}

/**
 * @brief タッチ点灯。
 */
static int LEDRing_touchImpl_()
{
	{
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("LEDR", 4);
		const uint8_t subtype = 9;
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
	}
	
	return 0; // OK
}


LEDRing::LEDRing() :
		subsystem_(ArduinoSubsystem::getInstance()),
		fps_(1)
{}

int LEDRing::show(bool async)
{
	if(async){
		showAsync_ = std::async(std::launch::async, LEDRing_showImpl_, frames_, fps_);
		return 0;
	}else{
		return LEDRing_showImpl_(frames_, fps_);
	}
}

int LEDRing::reset(bool async)
{
	if(async){
		resetAsync_ = std::async(std::launch::async, LEDRing_resetImpl_);
		return 0;
	}else{
		return LEDRing_resetImpl_();
	}
}

int LEDRing::set(bool async, uint8_t r_, uint8_t g_, uint8_t b_)
{
	if(async){
		resetAsync_ = std::async(std::launch::async, LEDRing_setImpl_, r_,  g_,  b_);
		return 0;
	}else{
		return LEDRing_setImpl_( r_,  g_,  b_);
	}
}

int LEDRing::setOne(bool async, uint8_t led_, uint8_t r_, uint8_t g_, uint8_t b_)
{
	if(async){
		resetAsync_ = std::async(std::launch::async, LEDRing_setOneImpl_, led_, r_,  g_,  b_);
		return 0;
	}else{
		return LEDRing_setOneImpl_(led_, r_,  g_,  b_);
	}
}

int LEDRing::touch(bool async)
{
	if(async){
		touchAsync_ = std::async(std::launch::async, LEDRing_touchImpl_);
		return 0;
	}else{
		return LEDRing_touchImpl_();
	}
}

int LEDRing::motion(bool async, uint8_t motion, uint8_t colVar, uint8_t r_, uint8_t g_, uint8_t b_)
{
	if(async){
		touchAsync_ = std::async(std::launch::async, LEDRing_motionImpl_, motion,  colVar,  r_,  g_,  b_);
		return 0;
	}else{
		return LEDRing_motionImpl_(motion,  colVar,  r_,  g_,  b_);
	}
}


}
