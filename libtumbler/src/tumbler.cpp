/*
 * @file tumbler.cpp
 * @brief Tumbler サブシステム制御ライブラリ共通機能の実装
 * @author Copyright (C) 2017 Fairy Devices Inc. http://www.fairydevices.jp/
 * @author Masato Fujino, created on: 2017/11/21 
 */

#include "tumbler/tumbler.h"

#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sstream>
#include <mutex>
#include <thread>
#include <wiringSerial.h>
#include <iostream>

namespace tumbler{

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
	c_status_ledringChange_.store(false);
	connectionOpen();
}

int ArduinoSubsystem::read(char* buf, int length)
{
	return ::read(serial_, buf, length);
}

int ArduinoSubsystem::write(const char* buf, int length)
{
	return ::write(serial_, buf, length);
}

int ArduinoSubsystem::dataAvail()
{
	return serialDataAvail(serial_);
}

int ArduinoSubsystem::sketchVersion()
{
	std::lock_guard<std::mutex> lock(global_lock_);
	char ack[8];
	write("VERC",4);
	const uint8_t subtype = 0;
	const uint8_t length  = 0;
	write(reinterpret_cast<const char*>(&subtype), 1);
	write(reinterpret_cast<const char*>(&length), 1);
	size_t readlen = read(ack, 8);
	if(readlen == 1 ){
		if(ack[0] == '1'){
			// バージョン問い合わせ関数未実装のスケッチの場合は全部 100 とする
			return 1000;
		}else{
			return -1;
		}
	}else if(readlen == 2){
		if(ack[1] == '1'){
			return static_cast<short>(ack[0]);
		}else{
			return -2;
		}
	}else{
		// 読み長さが異なる場合は異常
		return -3;
	}
}


void ArduinoSubsystem::connectionOpen()
{
	int baudrate = 19200;
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

}

