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

static int LEDRing_resetImpl_()
{
	char ack[8];
	int readlen = 0;
	{
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("LEDR",4);
		const uint8_t subtype = 0;
		const uint8_t length  = 0;
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&length), 1);
		readlen = subsystem.read(ack, 8);
	}
	if(readlen == 1 && ack[0] == '1'){
		return 0; // OK
	}else{
		return 1; // NG
	}
}

static int LEDRing_showImpl_(const std::vector<Frame>& frames, int fps)
{
	Timer tm;
	float std_wait_msec = 1000. / static_cast<float>(fps);
	char txdata[128];
	char ack[8];
	int readlen = 0;
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
			readlen = subsystem.read(ack, 1);
		}
		float t1 = tm.stop();
		float mod_wait_msec = std_wait_msec - t1;
		usleep(mod_wait_msec*1000);
	}
	if(readlen == 1 && ack[0] == '1'){
		return 0; // OK
	}else{
		return 1; // NG
	}
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



}



