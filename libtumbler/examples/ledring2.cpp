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
 * @class AzimuthFrame
 * @brief Tumbler 座標系における方位角（向かって右が 0 度、正面が 270 度）を指定した時、指定方位の LED を点灯させる機能を持つ。
 * libtumbler が提供している Frame クラスを継承している。
 * @details LED は 18 個しかないため、ちょうど 18 で割り切れる方位以外は、2 つの LED の明度のバランスを取ることで表現している
 */
class AzimuthFrame : public Frame
{

public:
	/**
	 * @brief コンストラクタ
	 * @param [in] 点灯させたい方向の方位角[0,360]
	 * @param [in] foreground 点灯させたい方向の LED 色
	 * @param [in] background その他の LED 色
	 */
	AzimuthFrame(
			const std::vector<int>& azimuths,
			const tumbler::LED& foreground,
			const tumbler::LED& background) :
		tumbler::Frame(background),
		azimuths_(azimuths.begin(), azimuths.end()),
		foreground_(foreground),
		background_(background)
	{
		// LED は 18 個あり、20 度ごとの角度で配置されている
		int node0Idx, node1Idx; // 反時計回りの始点と終点
		float node0, node1;     // 同明度
		for(size_t i=0;i<azimuths_.size();++i){
			// 値域対応
			int caz = azimuths_[i];
			if(360 <= caz){
				caz = caz - 360;
			}else if(caz < 0){
				caz = 360 + caz;
			}

			// 角度からセグメントを計算
			if(caz < 10 || 350 <= caz){
				// 端点処理
				node0Idx = 17;
				node1Idx = 0;
			}else{ // 10 -> 349
				// [10,29]->[20,39]=>1
				// [30,49]->[40,59]=>2
				// [330,349]->[340,359]=>17
				int segment = (caz+10) / 20;
				node0Idx = segment-1;
				node1Idx = segment;
			}

			// 明度バランス
			// 角度 = 0  -> ida = 10
			// 角度 = 5  -> ida = 15
			// 角度 = 9  -> ida = 19
			// 角度 = 10 -> ida =  0
			// 角度 = 15 -> ida =  5
			// 角度 = 17 -> ida =  7
			// 角度 = 20 -> ida = 10
			// ....
			int ida =  (caz+10) % 20;
			CalcBrightnessPair(ida, node0, node1);
			setLED(V2PMap(node0Idx), tumbler::LED(foreground.r_*node0, foreground.g_*node0, foreground.b_*node0));
			setLED(V2PMap(node1Idx), tumbler::LED(foreground.r_*node1, foreground.g_*node1, foreground.b_*node1));
		}
	}

private:

	/**
	 * @brief 所与の内分角に対して 2 つの LED ペアの明るさバランスを計算する
	 * @param [in] degree 内分角[0,19](度)
	 * @param [out] node0 片側の明度（反時計回りの始点側）
	 * @param [out] node1 片側の明度（反時計回りの終点側）
	 */
	void CalcBrightnessPair(int degree, float& node0, float& node1)
	{
		if(degree < 0 || 19 < degree){ // [0,19]度
			throw std::runtime_error("CalcBrightnessPair(), degree out of range.");
		}
		node0 = 1.0 - degree*0.05;
		node1 = degree*0.05;
	}

	/**
	 * @brief 基準座標系における仮想LEDIDと物理LED番号の対応
	 * @param [in] VID 仮想LEDID
	 * @return 物理LEDID
	 */
	int V2PMap(int VID)
	{
		switch (VID){
		case 0: return 4;
		case 1: return 3;
		case 2: return 2;
		case 3: return 1;
		case 4: return 0;
		case 5: return 17;
		case 6: return 16;
		case 7: return 15;
		case 8: return 14;
		case 9: return 13;
		case 10: return 12;
		case 11: return 11;
		case 12: return 10;
		case 13: return 9;
		case 14: return 8;
		case 15: return 7;
		case 16: return 6;
		case 17: return 5;
		default: return -1;
		}
	}

	std::vector<int> azimuths_;
	const tumbler::LED foreground_;
	const tumbler::LED background_;
};

/**
 * @brief 角度計算
 * @param [in] base 基準となる角度
 * @param [in] diff 基準となる角度からの差
 * @return 基準となる角度に対して、第二引数で指定した差を持つ角度
 */
int calcDiffDegree(int base, int diff)
{
	int d = base-diff;
	if(d < 0){
		return d+360;
	}else if(360 < d){
		return d-360;
	}else{
		return d;
	}
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

/**
 * @brief アニメーション定義の実装
 * @param [in] degree LED を点灯させたい方位角
 * @return アニメーションのためのフレーム群
 * @note AzimuthFrame クラスは単一のフレームを返す。libmimixfe では AzimuthFrame クラスの直接利用による単一フレームを用いているが
 * 本サンプルプログラムでは、それらを複数用いてアニメーションの例を定義してみた。
 */
std::vector<Frame> myAnimation(int degree)
{
	LED foregroundColor(0,0,255);
	AzimuthFrame af2({calcDiffDegree(degree,70), calcDiffDegree(degree,-70)}, foregroundColor, LED(40,40,40));
	AzimuthFrame af3({calcDiffDegree(degree,50), calcDiffDegree(degree,-50)}, foregroundColor, LED(30,30,30));
	AzimuthFrame af4({calcDiffDegree(degree,20), calcDiffDegree(degree,-20)}, foregroundColor, LED(20,20,20));
	AzimuthFrame af6({degree}, foregroundColor, LED(10,10,10));
	return {af2,af3,af4,af6};
}

int main(int argc, char** argv)
{
	LEDRing& ring = LEDRing::getInstance();
    ring.motion(false, 1, defaultPattern());

    std::cout << "角度を入力すると、角度の方向がアニメーション付きで点灯します" << std::endl << std::endl;
    std::cout << "プログラムを終了するときは Ctrl+C で終了してください" << std::endl;
    ring.setFPS(30);
    std::string input;
    while(true){
    	std::cout << "角度を[0,359]の範囲で入力してください" << std::endl;
    	std::getline(std::cin, input);
    	int degree = std::atoi(input.c_str());
    	if(!(0 <= degree && degree < 360)){
    		std::cout << "角度は[0,359]の範囲で入力してください" << std::endl;
    		continue;
    	}
		ring.setFrames(myAnimation(degree));
		ring.show(true); // 非同期で実行しています。このサンプルプログラムでは同期実行でも問題ありませんが、実際のプログラムでは、アニメーションの終了を待ちたくない場合があります。
		std::cout << "リセットするために、もう一度リターンキーを押してください" << std::endl;
		std::cin.get();
        ring.motion(false, 1, defaultPattern());
    }
}
