/*
 * @file tumbler.h
 * @brief Tumbler サブシステム制御ライブラリ共通ヘッダ
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_TUMBLER_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_TUMBLER_H_

#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#define DLL_LOCAL  __attribute__ ((visibility ("hidden")))

#include <vector>
#include <string>
#include <mutex>
#include <stdexcept>

namespace tumbler
{

	/**
	 * @class ArduinoSubsystemError
	 * @brief Arduino サブシステム内、もしくは通信で発生したエラー
	 */
	class DLL_PUBLIC ArduinoSubsystemError : public std::runtime_error
	{
	public:
		explicit ArduinoSubsystemError(int errorno) : runtime_error(errorstr(errorno)), errorno_(errorno) {}
		int errorno() const { return errorno_; }
		ArduinoSubsystemError(int errorno, const std::string& errstr) : runtime_error(errstr), errorno_(errorno) {}
	private:
		const std::string errorstr(int errorno);
		int errorno_;
	};

	/**
	 * @class ArduinoSubsystem
	 * @brief Arduino Subsystem へのシリアル通信路を保持するシングルトンクラスであり、送受信を排他制御する
	 */
	class ArduinoSubsystem
	{
	public:
		/**
		 * @brief Arduino Subsystem へのシリアル通信路を保持するシングルトンインスタンスを取得する
		 * @note 最初に取得されたときに通信路が開かれる。通信路を開くことができなかった場合は、ArduinoSubsystemError 例外が送出される。
		 * @return シングルトンインスタンス
		 */
		static ArduinoSubsystem& getInstance();

		/**
		 * @brief ロックを取得し Arduino Subsystem へのシリアル通信路から読み出す
		 * @param [out] buf 読み出しバッファ
		 * @param [in] length 読み出しバッファ長（byte）
		 * @return 実際の読み出し長（byte）
		 */
		int read(char* buf, int length);

		/**
		 * @brief ロックを取得し Arduino Subsystem へのシリアル通信路へ書き込む
		 * @param [out] buf 書き込みバッファ
		 * @param [in] length 書き込みバッファ長（byte）
		 * @return 実際の書き込み長（byte）
		 */
		int write(const char* buf, int length);

		/**
		 * @brief Arduino Subsystem をハードウェアリセットする
		 * @note アプリケーションから利用する必要は基本的にはないと想定される
		 */
		void hardReset();

		/**
		 * @brief read/write をまとめて外部からロックする
		 */
		std::mutex global_lock_;

	private:
		ArduinoSubsystem();
		~ArduinoSubsystem();
		ArduinoSubsystem(const ArduinoSubsystem&);
		ArduinoSubsystem &operator=(const ArduinoSubsystem&);
		void connectionOpen();
		void connectionClose();

		int serial_;
	};

	/**
	 * @class Timer
	 * \~english
	 * @brief Class for code block time mesurement. You can measure the elapsed time of any code blocks with using start() and stop() function of this class, stop() function returns the elapsed time from the time when start() function was called. This class has summation of the measured elapsed time periods, you can get the summation time by total() function.
	 * \~japanese-en
	 * @brief 時刻計測クラス. 時刻計測を行いたいブロックの前後に start() と stop() を組み合わせて使う。stop() は、start() が呼び出された時刻からの経過時間を返す. このクラスは、start() と stop() の複数回の呼び出しに対応する複数個の経過時間を積算している。積算経過時間は total() 関数により得られる。
	 */
	class Timer{
	public:
		Timer() : sum_microsec_(0), status_(false) {}

		/**
		 *\~english
		 * @brief Start time mesurement.
		 * \~japanese-en
		 * @brief 時刻計測を開始する
		 */
		void start()
		{
			start_ = std::chrono::system_clock::now();
			status_ = true; // 時刻計測の開始（タイマースタートのイメージ）
		}

		/**
		 *\~english
		 * @brief Stop time mesurement
		 * @return Elapsed time from start() function is called. [millisecond]
		 * \~japanese-en
		 * @brief 時刻計測を一時停止する
		 * @return 今回の計測区間の経過時間 [millisecond]
		 */
		float stop()
		{
			if(!status_){
				throw std::runtime_error("time is not started.");
			}
			auto end = std::chrono::system_clock::now();
			float msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
			float microsec = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
			sum_microsec_ += microsec;
			status_ = false;
			return msec;
		}

		/**
		 * \~english
		 * @brief Returns the sum of mesuament time periods.
		 * @return total mesuament time periods; (stop() - start()) + (stop()-start()) + ...
		 * \~japanese
		 * @brief 全 start()-stop() 間の合計経過時間を返す
		 * @return 合計経過時間
		 */
		int total(){ return static_cast<int>(sum_microsec_ / 1000.); }

	private:
		std::chrono::system_clock::time_point start_;
		float sum_microsec_;
		bool status_;
	};


}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_TUMBLER_H_ */
