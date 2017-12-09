#include <iostream>
#include <wiringSerial.h>
#include <signal.h>

volatile sig_atomic_t e_flag_ = 0;
void sig_handler_(int signum){ e_flag_ = 1; } // Ctrl+C のキャプチャ

int main(int argc, char** argv)
{
	signal(SIGINT, sig_handler_); // Ctrl+C のキャプチャ
		
	// 録音ファイルの準備（48kHz, 16bit 18ch PCM）
	FILE *outputfile = fopen("rec_48k-18ch.raw","w");
	if(outputfile == nullptr){
		std::cout << "Could not open output file" << std::endl;
		return 1;
	}
	
	// 録音デバイスを開く
	FILE *stream = fopen("/dev/ttyACM0","r");
	if(stream == nullptr){
		std::cout << "Could not open /dev/ttyACM0" << std::endl;
		return 1;
	}
	
	// 録音デバイスに録音開始命令を送る
	int control = serialOpen("/dev/ttyACM0",9600);
	serialPutchar(control, '1'); // 録音開始コマンド
	std::cout << "recording start..." << std::endl;

	// 録音デバイスからデータ受信
	const size_t chunk_size = 480*18; // 48kHz のとき 10 msec 相当 x 18ch
	short buf[chunk_size];
	while(e_flag_ == 0){
		fread(buf,sizeof(short),chunk_size,stream);
		fwrite(buf,sizeof(short),chunk_size,outputfile);
	}

	// 終了
	serialPutchar(control, '0'); // 録音終了コマンド
	serialClose(control);
	fclose(stream);
	fclose(outputfile);
	std::cout << "recording normally finished." << std::endl;
	return 0;
}
