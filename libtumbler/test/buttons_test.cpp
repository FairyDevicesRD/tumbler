/*
 * @file buttons_test.cpp
 * \~english
 * @brief Test program for buttns on top panel.
 * \~japanese
 * @brief トップパネルボタンのテストプログラム
 * \~ 
 * @author Masato Fujino, created on: Mar 7, 2019
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
#include <vector>
#include <unistd.h>
#include <fstream>
#include <string>
#include "tumbler/tumbler.h"
#include "tumbler/speaker.h"
#include "tumbler/buttons.h"

using namespace tumbler;

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

void ButtonStateFunc(std::vector<ButtonState> state)
{
	std::cout << "callback" << std::endl;
	int pushed = 0;
	for(int i=0;i<4;++i){
		if(state[i] == ButtonState::pushed_){
			std::cout << "pushed" << std::endl;
			pushed++;
		}else if(state[i] == ButtonState::none_){
			std::cout << "-" << std::endl;
		}
	}
	if(pushed == 4){
		std::vector<short> audio = FileReader("data/test1.raw");
		Speaker& spk = Speaker::getInstance();
		spk.batchPlay(audio, 44100, 0.05, Speaker::PlayBackMode::overwrite_);
	}
}

int main(int argc, char** argv)
{
	Buttons& buttons = Buttons::getInstance(ButtonStateFunc);

    buttons.start();
    sleep(10);
    buttons.stop();
    return 0;
}
