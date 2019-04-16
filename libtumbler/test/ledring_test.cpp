/*
 * @file ledring_test.cpp
 * @brief ledring.h の単体試験
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/22 
 */

#include <iostream>
#include <unistd.h>
#include "tumbler/tumbler.h"
#include "tumbler/ledring.h"

using namespace tumbler;

Frame rainbowPattern()
{
	  Frame f;
	  f.setLED(0, LED(255,0,0));
	  f.setLED(1, LED(255,0,0));
	  f.setLED(2, LED(255,0,0));
	  f.setLED(3, LED(255,165,0));
	  f.setLED(4, LED(255,165,0));
	  f.setLED(5, LED(255,165,0));
	  f.setLED(6, LED(0,255,0));
	  f.setLED(7, LED(0,255,0));
	  f.setLED(8, LED(0,255,0));
	  f.setLED(9, LED(0,255,255));
	  f.setLED(10, LED(0,255,255));
	  f.setLED(11, LED(0,255,255));
	  f.setLED(12, LED(0,0,255));
	  f.setLED(13, LED(0,0,255));
	  f.setLED(14, LED(0,0,255));
	  f.setLED(15, LED(128,0,128));
	  f.setLED(16, LED(128,0,128));
	  f.setLED(17, LED(128,0,128));
	  return f;
}

Frame defaultPattern(){
	Frame f;
	int r = 255;
	int g = 255;
	int b = 255;
	float a = 0.4;
	float c = 0.6;
	f.setLED(4, LED(r, g, b));
	f.setLED(3, LED(r, g, b));
    f.setLED(2, LED(r*a*a,g*a*a,b*c));
    f.setLED(1, LED(r*a*a*a,g*a*a*a,b*c*c));
    f.setLED(0, LED(r*a*a*a*a,g*a*a*a*a,b*c*c*c));
    return f;
}

int main(int argc, char** argv)
{
    {
        // 青点灯
		Frame frame;
		for(int i=0;i<18;++i){
			frame.setLED(i, LED(0,0,255));
		}
		LEDRing& ring = LEDRing::getInstance();
		ring.addFrame(frame);
		ring.show(false);
		sleep(3); // 表示 3 秒
    }
    {
		// 外部制御アニメーション点灯（R->G->B の順）
		std::vector<Frame> frames(3);
		for(int i=0;i<18;++i){
			frames[0].setLED(i, LED(255,0,0));
			frames[1].setLED(i, LED(0,255,0));
			frames[2].setLED(i, LED(0,0,255));
		}
		LEDRing& ring = LEDRing::getInstance();
		ring.setFrames(frames);
		ring.setFPS(1);
		ring.show(false); // 同期（アニメーション期間中処理を戻さない）
    }
    {
    	// 外部制御アニメーション点灯（回転）
		LEDRing& ring = LEDRing::getInstance();
		ring.clearFrames();
		LED background(50,50,50);
		for(int rot = 0; rot < 5; ++rot){ // 5 回転
			for(int i=0;i<18;++i){
				Frame frame(background);
				frame.setLED(i, LED(0,0,255));
				ring.addFrame(frame);
			}
		}
		ring.setFPS(30);
    	ring.show(false);
    }
    {
    	// 内部制御アニメーション点灯（フレーム外部指定、固定点灯から固定速回転へ）
    	LEDRing& ring = LEDRing::getInstance();
    	ring.motion(false, 0, rainbowPattern());
    	sleep(3); // 内部制御であるので、ウェイトしないと抜けてしまう（表示 3 秒）
    	ring.motion(false, 1, rainbowPattern());
    	sleep(5); // 内部制御であるので、ウェイトしないと抜けてしまう（表示 5 秒）
    }
    {
    	// 内部制御アニメーション点灯（フレーム外部指定、固定速逆回転）
    	LEDRing& ring = LEDRing::getInstance();
    	ring.motion(false, 2, defaultPattern());
    	sleep(3);
    }

	LEDRing& ring = LEDRing::getInstance();
	ring.reset(false); // 非同期でリセットし終了
	return 0;
}


