/*
 * @file tumbler.cpp
 * @brief Tumbler サブシステム制御ライブラリ共通機能の実装
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */

#include "tumbler/tumbler.h"

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sstream>
#include <wiringSerial.h>
#include <termios.h>
#include <mutex>
#include <future>
#include <thread>
#include <string.h>

#include "tumbler/bme.h"
#include "tumbler/touch.h"

namespace tumbler{

#define CALLBACK_STOP	0
#define CALLBACK_RUN	1
#define CALLBACK_WAIT	2
	
/**
 * @brief シリアル受信スレッドの登録.
 * @param callbackFnc: タッチセンサー受信時のコールバック関数
 */
static int serialCallBack(void (*callbackFnc)(char*,int))
{
	int pos = 0;
	char buf[2];
	static char readBuf[255];
	ArduinoSubsystem & subsystem_ = ArduinoSubsystem::getInstance();
	while(subsystem_.callBackState != CALLBACK_STOP)
	{
		if(subsystem_.callBackState == CALLBACK_WAIT){
			usleep(10000);
			continue;
		}
  
		while(subsystem_.serialAvail()){
			subsystem_.read(buf,1);
			readBuf[pos++] = buf[0];

			if(buf[0] == '\n' || pos > 100){
				// 1行分受信完了.
				readBuf[pos] = '\0';

				if(readBuf[0] == 'B' && readBuf[1] == 'M'){
					// 環境センサー.
					BMEControl &bme = BMEControl::getInstance();
					bme.update(readBuf, pos);
				}
				else if(readBuf[0] == 'T' && (readBuf[1] >= 'A' && readBuf[1] <= 'Z')){
					// タッチセンサー.
					TouchSensor &touch = TouchSensor::getInstance();
					touch.update(readBuf, pos);
					callbackFnc(readBuf, pos);
				}
				else{
					std::cout << "[serial] " << readBuf << std::endl;
				}

				pos = 0;
				break;
			}
		}
		usleep(1000);
	}
	return 0;
}

/**
 * @brief シリアル受信スレッドの開始
 * @param callbackFnc: タッチセンサー受信時のコールバック関数
 */
int ArduinoSubsystem::startCallBack(void (*callbackFnc)(char*,int))
{
	if(callBackState == CALLBACK_STOP){
		callBackState = CALLBACK_RUN;
		callbackAsync_ = std::async(std::launch::async, serialCallBack, callbackFnc);
	}
	else{
		// 既に動作している場合は一度止めて再起動させる。
		stopCallBack();
		callBackState = CALLBACK_RUN;
		callbackAsync_ = std::async(std::launch::async, serialCallBack, callbackFnc);
	}
	return 0;

}

/**
 * @brief シリアル受信スレッドの終了
 */
int ArduinoSubsystem::stopCallBack()
{
  if(callBackState != CALLBACK_STOP){
	callBackState = CALLBACK_STOP;
	callbackAsync_.get();
  }
  return 0;
}

/**
 * @brief シリアル受信スレッドの一時停止(仮)
 */
int ArduinoSubsystem::waitCallBack()
{
	if(callBackState == CALLBACK_RUN)
	{
	  callBackState = CALLBACK_WAIT;
	}
	flush();
  return 0;
}

/**
 * @brief シリアル受信スレッドの再開(仮)
 */
int ArduinoSubsystem::restartCallBack()
{
	if(callBackState != CALLBACK_STOP)
	{
	  callBackState = CALLBACK_RUN;
	}
  return 0;
}

/**
 * @brief 空のコールバック関数.
 */
void callbackFncDummy(char* data,int size)
{
}

const std::string ArduinoSubsystemError::errorstr(int errorno)
{
	std::stringstream s;
	switch(errorno){
	case 100:
		s << "Could not connect to Arduino Subsystem via /dev/ttyAMA0 (";
		break;
	default:
		s << "Not defined in errorstr() function (";
		break;
	}
	s << errorno << ")";
	syslog(LOG_ERR, "%s", s.str().c_str());
	return s.str();
}

ArduinoSubsystem& ArduinoSubsystem::getInstance()
{
	static ArduinoSubsystem instance;
	return instance;
}

ArduinoSubsystem::~ArduinoSubsystem()
{
	stopCallBack();
	connectionClose();
}

void ArduinoSubsystem::hardReset()
{
	connectionClose();
	system("/bin/stty --file /dev/ttyAMA0 -hupcl");
	connectionOpen();
}

ArduinoSubsystem::ArduinoSubsystem()
{
	openlog("libtumbler", LOG_PID, LOG_USER);
	connectionOpen();
	startCallBack(callbackFncDummy);
}

int ArduinoSubsystem::read(char* buf, int length)
{
	return ::read(serial_, buf, length);
}

int ArduinoSubsystem::readline(char* buf, int length)
{
	char *tmpP = buf;
	int i = 0;
	while(i < length){
		::read(serial_, tmpP, 1);
		if(*tmpP == '\n' || *tmpP == '\0')	break;
		i++; tmpP++;
	}
	return i;
}

int ArduinoSubsystem::write(const char* buf, int length)
{
	return ::write(serial_, buf, length);
}

void ArduinoSubsystem::connectionOpen()
{
	int baudrate = 19200;
//	int baudrate = 38400;
	serial_ = serialOpen("/dev/ttyAMA0",baudrate);
	if(serial_ < 0){
		throw ArduinoSubsystemError(100);
	}
	syslog(LOG_INFO, "Arduino subsystem connection is opened with /dev/ttyAMA0 at %d bps", baudrate);
}

void ArduinoSubsystem::connectionClose()
{
	serialFlush(serial_);
	serialClose(serial_);
	syslog(LOG_INFO, "Arduino subsystem connection is closed");
}

void ArduinoSubsystem::flush()
{
	serialFlush(serial_);
	tcflush(serial_, TCIOFLUSH);
}

}

