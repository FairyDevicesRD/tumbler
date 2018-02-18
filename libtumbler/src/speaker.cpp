/*
 * @file speaker.cpp
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

#include "tumbler/speaker.h"
#include <unistd.h>
#include <mutex>
#include <future>
#include <thread>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace tumbler
{

Speaker& Speaker::getInstance()
{
	static Speaker instance;
	return instance;
}

Speaker::Speaker()
{
	rate_.store(44100);
	init(rate_.load());
}

Speaker::~Speaker()
{
	close();
}

void Speaker::init(int rate)
{
	rate_.store(rate);
	state_.store(false);
	std::unique_lock<std::mutex> lock(mutex_);
	int errorno = snd_pcm_open(&pcm_handle_, device_name_.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
	if(errorno < 0){
		std::stringstream s;
		s << "Could not open speaker with ALSA; " << snd_strerror(errorno) << "(" << errorno << ")" << std::endl;
		throw std::runtime_error(s.str());
	}
	snd_pcm_hw_params_alloca(&pcm_params_);
	snd_pcm_hw_params_any(pcm_handle_, pcm_params_);
	errorno = snd_pcm_hw_params_set_access(pcm_handle_, pcm_params_, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(errorno < 0){
		std::stringstream s;
		s << "Could not set interleaved mode with ALSA " << snd_strerror(errorno) << "(" << errorno << ")" << std::endl;
		throw std::runtime_error(s.str());
	}
	errorno = snd_pcm_hw_params_set_format(pcm_handle_, pcm_params_, SND_PCM_FORMAT_S16_LE);
	if(errorno < 0){
		std::stringstream s;
		s << "Could not set format with ALSA; " << snd_strerror(errorno) << "(" << errorno << ")" << std::endl;
		throw std::runtime_error(s.str());
	}
	// 左チャネルはスピーカー用、右チャネルは BT/USB オーディオサブシステムへの入力となることに留意
	errorno = snd_pcm_hw_params_set_channels(pcm_handle_, pcm_params_, 2);
	if(errorno < 0){
		std::stringstream s;
		s << "Could not set channels number with ALSA; " << snd_strerror(errorno) << "(" << errorno << ")" << std::endl;
		throw std::runtime_error(s.str());
	}
	// 再生サンプリングレートは初期値 play コール毎に変えられる
	errorno = snd_pcm_hw_params_set_rate(pcm_handle_, pcm_params_, rate, 0);
	if(errorno < 0){
		std::stringstream s;
		s << "Could not set rate with ALSA; " << snd_strerror(errorno) << "(" << errorno << ")" << std::endl;
		throw std::runtime_error(s.str());
	}
	errorno = snd_pcm_hw_params(pcm_handle_, pcm_params_);
	if(errorno < 0){
		std::stringstream s;
		s << "Could not set hw parameters with ALSA; " << snd_strerror(errorno) << "(" << errorno << ")" << std::endl;
		throw std::runtime_error(s.str());
	}
}

void Speaker::close()
{
	snd_pcm_drain(pcm_handle_);
	snd_pcm_close(pcm_handle_);
}

void Speaker::batchPlay(const std::vector<short>& audio, int rate, float volume, Speaker::PlayBackMode mode)
{
	state_.store(true);
	auto th = std::thread([&](int rate, float volume, Speaker::PlayBackMode mode){
		std::cout << "prepare audio play" << std::endl;
		if(rate_.load() != rate){
			close();
			init(rate);
		}
		short max_value = 0;
		for(size_t i=0;i<audio.size();++i){
			if(max_value < std::abs(audio[i])) max_value = std::abs(audio[i]);
		}
		std::cout << "mv=" << max_value << std::endl;
		size_t size = audio.size()*2;
		std::vector<short> stereoaudio(size*2,0);
		int j=0;
		for(size_t i=0;i<size;i+=2){
			stereoaudio[i] = static_cast<float>(audio[j]) / static_cast<float>(max_value) * 32766 * volume;
			++j;
		}
		std::cout << "play audio, " << stereoaudio.size() << std::endl;
		snd_pcm_prepare(pcm_handle_);
		snd_pcm_writei(pcm_handle_, stereoaudio.data(), stereoaudio.size()/2);
		state_.store(false);
		std::cout << "played audio, " << stereoaudio.size() << std::endl;
	},  rate, volume, mode);
	th.detach();
}

}

