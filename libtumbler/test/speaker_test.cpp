/*
 * @file speaker_test.cpp
 * \~english
 * @brief Test for speaker
 * \~japanese
 * @brief スピーカーテスト
 * \~
 * @copyright Copyright 2018 Fairy Devices Inc. http://www.fairydevices.jp/
 * @copyright Apache License, Version 2.0
 * @author Masato Fujino, created on: 2018/02/19
 *
 * Copyright 2018 Fairy Devices Inc. http://www.fairydevices.jp/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unistd.h>

#include "tumbler/tumbler.h"
#include "tumbler/speaker.h"

std::vector<short> FileReader(const std::string& filename)
{
	std::vector<short> filedata;
	std::ifstream inputfs(filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
	if(!inputfs.is_open()){
		throw std::runtime_error("FileReader: Could not open file");
	}
	size_t inputsize = inputfs.tellg();
	filedata.resize(inputsize/2);
	inputfs.seekg(0, std::ios::beg);
	inputfs.read(reinterpret_cast<char*>(&filedata[0]),inputsize);
	inputfs.close();
	return filedata;
}

int main(int argc, char** argv)
{
	using namespace tumbler;
	std::vector<short> audio = FileReader("data/test1.raw");
	std::vector<short> ad;
	for(uint i=0;i<audio.size();++i){
		ad.push_back(audio[i]);
	}
	for(uint i=0;i<audio.size();++i){
		ad.push_back(audio[i]);
	}
	for(uint i=0;i<audio.size();++i){
		ad.push_back(audio[i]);
	}
	{
		std::cout << "Initialize ALSA 1" << std::endl;
		Speaker& spk = Speaker::getInstance();
		std::cout << "Batch Play..." << std::endl;
		spk.batchPlay(ad, 44100, 0.05, Speaker::PlayBackMode::overwrite_);
		std::cout << "Batch End..." << std::endl;
		while(spk.state()){
			usleep(1000*100);
		}
	}
	sleep(10);
	{
		std::cout << "Initialize ALSA 2" << std::endl;
		Speaker& spk = Speaker::getInstance();
		std::cout << "Batch Play..." << std::endl;
		spk.batchPlay(ad, 44100, 0.05, Speaker::PlayBackMode::overwrite_);
		std::cout << "Batch End..." << std::endl;
		while(spk.state()){
			usleep(1000*100);
		}
	}

	return 0;
}

