/*
 * @file speaker.h
 * \~english
 * @brief Audio play back with ALSA
 * \~japanese
 * @brief ALSA による音声再生ラッパー
 * \~
 * @copyright Copyright 2018 Fairy Devices Inc. http://www.fairydevices.jp/
 * @copyright Apache License, Version 2.0
 * @author Masato Fujino, created on: 2018/02/18
 *
 * Copyright 2018 Fairy Devices Inc. http://www.fairydevices.jp/
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
#ifndef LIBTUMBLER_INCLUDE_TUMBLER_SPEAKER_H_
#define LIBTUMBLER_INCLUDE_TUMBLER_SPEAKER_H_

#include "tumbler/tumbler.h"

#include <memory>
#include <future>
#include <vector>
#include <alsa/asoundlib.h>

namespace tumbler{

/**
 * @class Speaker
 * @brief Speaker を保持するシングルトンクラス
 * @todo 全面書き換え
 * @attention 全面書き換えされます
 */
class DLL_PUBLIC Speaker
{
public:
	/**
	 * @class PlayBackMode
	 * @brief プレイバックモード。再生されている状態で重複した再生命令に対する挙動
	 */
	enum class PlayBackMode
	{
		normal_, //!< 再生終了を待って再生
		overlay_, //!< 重ねて再生
		overwrite_, //!< 再生停止して新たに再生
	};

	static Speaker& getInstance();

	/**
	 * @brief モノラル音声を再生する
	 * @param [in] audio 再生したいモノラル音声データ
	 * @param [in] rate サンプリングレート
	 * @param [in] volume 再生ボリューム[0,1]
	 * @param [in] mode プレイバックモード
	 */
	void batchPlay(const std::vector<short>& audio, int rate, float volume, Speaker::PlayBackMode mode);

	/**
	 * @brief 音声再生中であるかを返す
	 * @return true if on play.
	 */
	bool state(){ return state_.load(); }

private:

	const std::string device_name_ = "default";
	void init(int rate);
	void close();

	Speaker();
	Speaker(const Speaker&);
	~Speaker();
	Speaker &operator=(const Speaker&);

    std::mutex mutex_;
	snd_pcm_t* pcm_handle_;
	snd_pcm_hw_params_t* pcm_params_;
	std::atomic<int> rate_;
	std::atomic<bool> state_;

};

}

#endif /* LIBTUMBLER_INCLUDE_TUMBLER_SPEAKER_H_ */
