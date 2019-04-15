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
		const uint8_t subtype = 0; // v1.0 ではデフォルト回転、v1.1 から消灯へ
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
			const uint8_t subtype = 8; // 外部制御アニメーションモード
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

static int LEDRing_motionImpl_(uint8_t motion, const Frame& frame)
{
	char txdata[128];
	char ack[8];
	int readlen = 0;
	const uint8_t data_length = frame.toDataForTx(txdata);
	const uint8_t comm_length = data_length + 1;
	{
		ArduinoSubsystem& subsystem = ArduinoSubsystem::getInstance();
		std::lock_guard<std::mutex> lock(subsystem.global_lock_);
		subsystem.write("LEDR",4);
		const uint8_t subtype = 1; // v1.1 から新設、組み込みアニメーションモード
		subsystem.write(reinterpret_cast<const char*>(&subtype), 1);
		subsystem.write(reinterpret_cast<const char*>(&comm_length), 1);
		subsystem.write(reinterpret_cast<const char*>(&motion), 1);
		subsystem.write(txdata, static_cast<int>(data_length));
		readlen = subsystem.read(ack, 1);
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
	clearFrames(); // v1.1 から追加
	Frame frame;
	for(int i=0;i<Frame::k_num_leds_;++i){
		currentFrame_ = frame; // @TODO クリアする位置を適正化せよ
	}
	if(async){
		resetAsync_ = std::async(std::launch::async, LEDRing_resetImpl_);
		return 0;
	}else{
		return LEDRing_resetImpl_();
	}
}

int LEDRing::motion(bool async, uint8_t animationPattern, const Frame& frame)
{
	if(async){
		motionAsync_ = std::async(std::launch::async, LEDRing_motionImpl_, animationPattern, frame);
		return 0;
	}else{
		return LEDRing_motionImpl_(animationPattern, frame);
	}
}

int LEDRing::motion(bool async, uint8_t animationPattern, uint8_t position, uint8_t r, uint8_t g, uint8_t b)
{
	// ブランチ間互換性維持のために組み込み旧定義をこちら側に取り出して再定義する
	Frame frame;
	if(position == 0){
		float a = 0.4;
		float c = 0.6;
		frame.setLED(4, LED(r, g, b));
		frame.setLED(3, LED(r, g, b));
        frame.setLED(2, LED(r*a*a,g*a*a,b*c));
        frame.setLED(1, LED(r*a*a*a,g*a*a*a,b*c*c));
        frame.setLED(0, LED(r*a*a*a*a,g*a*a*a*a,b*c*c*c));
	}else if(position == 1){
		for(int i=0;i<Frame::k_num_leds_;i+=3){
			frame.setLED(i, LED(r,g,b));
		}
	}else if(position == 2){
		for(int i=0;i<Frame::k_num_leds_;i+=6){
			frame.setLED(i, LED(r,g,b));
		}
	}
	return motion(async, animationPattern, frame);
}

// deprecated
int LEDRing::set(bool async, uint8_t r, uint8_t g, uint8_t b)
{
	// 基本設計準拠の API 実装への影響を与えないように、組み込みアニメーションモードを利用して実装する
	Frame frame;
	for(int i=0;i<Frame::k_num_leds_;++i){
		frame.setLED(i, LED(r,g,b));
	}
	return motion(async, static_cast<uint8_t>(0), frame); // 全点灯固定
}

// deprecated
int LEDRing::setOne(bool async, uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
	// 基本設計準拠の API 実装への影響を与えないように、組み込みアニメーションモードを利用して実装する
	currentFrame_.setLED(index, LED(r, g, b));
	return motion(async, static_cast<uint8_t>(0), currentFrame_); // １点点灯固定
}



}



