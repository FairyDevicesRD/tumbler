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
	 * @brief LED リングの現在の点灯状態を取得する
	 * @details LEDRing クラスに登録されているフレーム定義ではなく、実際に LED リング上で点灯している最終のフレーム状態を返す。アニメーション
	 * @return LED リングの点灯状態
	 */
	Frame getCurrentFrame() const { return currentFrame_; }

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
	 * @brief LED リングをリセットする（LED リングを消灯し、本クラスに登録されているフレームを clearFrames() する）。
	 * @param [in] async true の場合リセット命令を非同期的に実行する。非同期実行の場合、処理の完了を待たずにこの関数は 0 を返す。
	 * @return リセット成功の場合 0、失敗の場合 1 が返される
	 */
	int reset(bool async);

	/**
	 * @brief シングルフレームを指定し、LED リングを Arduino サブシステムに組み込まれた特定のアニメーションパターンで直接点灯させる。
	 * @param [in] async true の場合、点灯命令を非同期的に実行する。非同期実行の場合、処理の完了を待たずにこの関数は 0 を返す。
	 * @param [in] animationPattern 組み込みアニメーションパターンの指定（0:停止、1:回転、2:逆回転）
	 * @param [in] frame フレーム
	 * @return 成功の場合 0 を返す
	 */
	int motion(bool async, uint8_t animationPattern, const Frame& frame);

	/**
	 * @brief 組み込み点灯位置と色（単一色）を指定し、LED リングを Arduino サブシステムに組み込まれた特定の点灯パターンで直接点灯させる。
	 * @details 任意の点灯位置と複数色を指定したいときは、もうひとつのオーバーロード関数を利用してください。この関数は、ブランチ間互換性維持のためのユーティリティ関数です。
	 * @param [in] async true の場合、点灯命令を非同期的に実行する。非同期実行の場合、処理の完了を待たずにこの関数は 0 を返す。
	 * @param [in] animationPattern 組み込みアニメーションパターンの指定（0:停止、1:回転、2:逆回転）
	 * @param [in] position 組み込み点灯位置の指定（0:部分点灯、1:6点点灯、2:3点点灯）
	 * @param [in] r 点灯色（赤）
	 * @param [in] g 点灯色（緑）
	 * @param [in] b 点灯色（青）
	 * @return 成功の場合 0 を返す
	 */
	int motion(bool async, uint8_t animationPattern, uint8_t position, uint8_t r, uint8_t g, uint8_t b);

	/**
	 * @brief ブランチ間互換性維持のためのユーティリティ関数
	 * @deprecated この関数は、１年程度の互換性猶予期間を以て、次期アップデートで廃止される可能性があります。利用しないことを推奨します。
	 */
	int set(bool async, uint8_t r, uint8_t g, uint8_t b);

	/**
	 * @brief ブランチ間互換性維持のためのユーティリティ関数
	 * @deprecated この関数は、１年程度の互換性猶予期間を以て、次期アップデートで廃止される可能性があります。利用しないことを推奨します。
	 */
	int setOne(bool async, uint8_t index, uint8_t r, uint8_t g, uint8_t b);

private:

	LEDRing();
	LEDRing(const LEDRing&);
	LEDRing &operator=(const LEDRing&);
	ArduinoSubsystem& subsystem_;
	std::vector<Frame> frames_;
	int fps_;
	std::future<int> resetAsync_;
	std::future<int> showAsync_;
	std::future<int> motionAsync_;
	Frame currentFrame_;
};

}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_LEDRING_H_ */
