/*
 * @file ledring.cpp
 * \~english
 * @brief Example program for controlling LED ring.
 * \~japanese
 * @brief LED リングの制御例
 * \~ 
 * @author Masato Fujino, created on: Apr 16, 2019
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

#include <iostream>
#include <unistd.h>
#include "tumbler/tumbler.h"
#include "tumbler/ledring.h"

using namespace tumbler;

/**
 * @brief サンプルプログラムで用いる虹色を再現したフレームを返す関数、18 個の LED で 7 色のグラデーションとなるように手で色指定した。
 * @return フレーム（18 個の LED の色指定が為されたもの）
 */
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

/**
 * @brief デフォルトパターンを返す関数、典型的に利用される関数です。
 * @details このパターンは Arduino Sketch にも内蔵されており、電源オフからオンへの切り替わり時点で OS が起動する前の時点で点灯されるパターン。reset 関数により消灯されるため、
 * デフォルトパターンでの再点灯は、内部制御点灯により libtumbler 側から改めて命令する必要があります。内蔵されたパターンの定義は、以下にあります。
 * @see https://github.com/FairyDevicesRD/tumbler/blob/be436908177daf22df32427c245034223300d102/arduino/sketch/LEDRing.h#L198
 * @return フレーム
 */
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
        // LED リング全体を青点灯
		Frame frame(LED(0,0,255));
		LEDRing& ring = LEDRing::getInstance();
		ring.addFrame(frame);
		ring.show(false); // async = false として、命令送信を待ちますが、通常は待つ必要はありません。このサンプルプログラムでは、連続的に LED 制御を行うため命令を同期的に行っています。
		sleep(3); // 3 秒間 LED リングを点灯します。
		ring.reset(false); // 消灯します
		sleep(1); // 1 秒間消灯状態を維持します
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
		ring.setFPS(1); // 要求 FPS = 1 なので、1 秒間に 1 フレーム表示されます。今 LED リングには 3 フレーム登録されているので、アニメーション全体には 3 秒間掛かります。
		ring.show(false); // 同期（アニメーション期間中処理を戻さない）、すなわち 3 秒後に処理が戻ります。
		sleep(1);
    }

    {
    	// 外部制御アニメーション点灯（回転）
		LEDRing& ring = LEDRing::getInstance();
		ring.clearFrames(); // 登録されているフレームをクリアします。クリア時点では LED リングは消灯されないことに留意してください。
		LED background(50,50,50);
		for(int rot = 0; rot < 5; ++rot){ // 5 回転分のフレームを登録します
			for(int i=0;i<18;++i){
				Frame frame(background);
				frame.setLED(i, LED(0,255,255));
				ring.addFrame(frame);
			}
		}
		ring.setFPS(30); // 要求 FPS = 30 なので、1 秒間に 30 フレーム表示されることが要求されています（実際のフレームレートが 30 であることを保証していません）。
    	ring.show(false);
    	sleep(1);
    }

    {
    	// 内部制御アニメーション点灯（フレーム外部指定、固定点灯から固定速回転へ）
    	LEDRing& ring = LEDRing::getInstance();
    	ring.motion(false, 0, rainbowPattern());
    	sleep(5); // 内部制御であるので、ウェイトしないと抜けてしまう（表示 3 秒）
    	ring.motion(false, 1, rainbowPattern());
    	sleep(5); // 内部制御であるので、ウェイトしないと抜けてしまう（表示 5 秒）
    	ring.motion(false, 2, rainbowPattern());
    	sleep(5); // 内部制御であるので、ウェイトしないと抜けてしまう（表示 5 秒）
    }

    {
    	// 内部制御アニメーション点灯（フレーム外部指定、固定速逆回転）
    	LEDRing& ring = LEDRing::getInstance();
    	ring.motion(false, 2, defaultPattern());
    	sleep(5);
    }

	LEDRing& ring = LEDRing::getInstance();
	ring.reset(false); // 同期リセット（消灯）して終了します
	return 0;
}
