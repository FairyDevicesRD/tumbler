/*
 * @file ledring.h
 * @brief LED リングの制御クラス
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_LEDRING_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_LEDRING_H_

#include "tumbler/tumbler.h"

#include <memory>
#include <future>
#include <vector>

namespace tumbler{

/**
 * @class LED
 * @brief LED 色定義（1 個の LED の RGB 値の定義）
 */
class DLL_PUBLIC LED
{
public:
	LED() : r_(0), g_(0), b_(0) {}
	LED(int r,int g, int b) : r_(r), g_(g), b_(b) {}
	LED(const LED& e) { r_ = e.r_; g_ = e.g_; b_ = e.b_; }
	LED& operator*=(float alpha)
	{
		r_ *= alpha;
		g_ *= alpha;
		b_ *= alpha;
		return *this;
	}
	void setColor(int r, int g, int b)
	{
		r_ = r;
		g_ = g;
		b_ = b;
	}
	int r_, g_, b_;
};

class DLL_PUBLIC Frame
{
public:
	Frame() {}
	/**
	 * @brief 単色のバックグラウンドカラーを設定するコンストラクタ
	 * @param [in] background バックグラウンド LED 色定義
	 */
	explicit Frame(const LED& background);
	Frame(const Frame& f);

	/**
	 * @brief フレームに LED 色定義をセットする
	 * @param [in] index LED 番号 [0,17]
	 * @param [in] led LED 色定義
	 */
	void setLED(int index, const LED& led) { leds_[index] = led; }

	/**
	 * @brief 指定したインデックスの LED 色定義を取得する
	 * @param [in] index LED 番号 [0,17]
	 * @return LED 色定義
	 */
	LED getLED(int index) const { return leds_[index]; }

	/**
	   @brief フレーム内容を送信データに書き換える.
	   @note 消灯の LED についてはデータを送信する必要がないことに留意する.
	   @param [out] 送信用データ
	   @return 送信用データ長
	 */
	uint8_t toDataForTx(char* data) const;

	static const int k_num_leds_ = 18;
	LED leds_[k_num_leds_];
};

/**
 * @class LEDRing
 * @brief LED リングを保持するシングルトンクラス。
 */
class DLL_PUBLIC LEDRing
{
public:
	static LEDRing& getInstance();

	/**
	 * @brief LED リングに描画フレームを追加する
	 * @param [in] frame フレーム
	 */
	void addFrame(const Frame& frame){ frames_.push_back(frame); }

	/**
	 * @brief LED リングに描画フレーム群をセットする
	 * @param [in] frames 描画フレーム群
	 */
	void setFrames(const std::vector<Frame>& frames){ frames_ = frames; }

	/**
	 * @brief LED リングにセットされた描画フレーム群を取得する
	 * @return セットされた描画フレーム群
	 */
	std::vector<Frame> getFrames() const { return frames_; }

	/**
	 * @brief LED リングにセットされた描画フレームをクリアする
	 */
	void clearFrames() { frames_.clear(); }

	/**
	 * @brief 描画 FPS（Frame Per Second）をセットする
	 * @note セットされた FPS は要求 FPS であり、大きすぎる FPS は再現されない
	 * @param [in] fps FPS
	 */
	void setFPS(int fps){ fps_ = fps; }

	/**
	 * @brief 描画 FPS を取得する
	 * @return FPS
	 */
	int getFPS() const { return fps_; }

	/**
	 * @brief セットされた描画フレーム群、FPS で実際に点灯実行する
	 */
	int show(bool async);

	/**
	 * @brief LED リングをリセットし、初期表示状態に戻す
	 * @param [in] async true の場合リセットを非同期的に実行する。非同期実行の場合、この関数は 0 を返しリセットの完了を待たずに処理を返す。
	 * @return リセット成功の場合 0、失敗の場合 1 が返される
	 */
	int reset(bool async);

	/**
	 * @brief LED リングをリセットし、任意の色で点灯させる
	 * @param [in] async true の場合リセットを非同期的に実行する。非同期実行の場合、この関数は 0 を返しリセットの完了を待たずに処理を返す。
	 * @return リセット成功の場合 0、失敗の場合 1 が返される
	 */
	int set(bool async, uint8_t r_, uint8_t g_, uint8_t b_);
	
	/**
	 * @brief LED リングの任意の1点を点灯させる
	 * @param [in] async true の場合リセットを非同期的に実行する。非同期実行の場合、この関数は 0 を返しリセットの完了を待たずに処理を返す。
	 * @return リセット成功の場合 0、失敗の場合 1 が返される
	 */
	int setOne(bool async, uint8_t led_, uint8_t r_, uint8_t g_, uint8_t b_);
	
	/**
	 * @brief LED リングをタッチセンサーに合わせて点灯させる
	 * @param [in] async true の場合リセットを非同期的に実行する。非同期実行の場合、この関数は 0 を返しリセットの完了を待たずに処理を返す。
	 */
	int touch(bool async);
  
	/**
	 * @brief LED リングをArduino内臓の動作パターンで点灯させる
	 * @param [in] motion 0:停止、1:回転、2:逆回転
	 * @param [in] colVar 0:部分点灯、1:6点点灯、2:3点点灯
	 */
	int motion(bool async, uint8_t motion, uint8_t colVar, uint8_t r_, uint8_t g_, uint8_t b_);
	
private:

	LEDRing();
	LEDRing(const LEDRing&);
	LEDRing &operator=(const LEDRing&);
	ArduinoSubsystem& subsystem_;
	std::vector<Frame> frames_;
	int fps_;
	std::future<int> resetAsync_;
	std::future<int> showAsync_;
	std::future<int> touchAsync_;
};

}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_LEDRING_H_ */
