namespace sonar{

	typedef struct ColorStruct
	{
		ColorStruct(): r_(0), g_(0), b_(0) {}
		ColorStruct(uint8_t r, uint8_t g, uint8_t b) : r_(r), g_(g), b_(b) {}
		uint8_t r_;
		uint8_t g_;
		uint8_t b_;
	} Color;

	typedef struct FrameBufferStruct
	{
		FrameBufferStruct(){}
		void incr()
		{
			Color c17 = led_[17];
			for(int i=17;i>0;i--){
				led_[i] = led_[i-1]; // LED17<-LED16, LED16<-LED15, ... LED1<-LED0, 
			}
			led_[0] = c17; // LED0<-LED17(backup)
		}		
		Color led_[18];
	} FrameBuffer;
	
	/**
	 * @class LEDRing
	 * @brief LED リングを表現するクラス
	 */
	class LEDRing : public Module
	{
	public:

		/**
		 * @brief C'tor
		 * @param [in] 明るさを周辺明度に合わせて自動調整する
		 */
		LEDRing(bool auto_luminance_adaptation) :
			Module("LEDRing"),
			auto_luminance_adaptation_(auto_luminance_adaptation),
			mode_(0)
		{}


		int8_t init() override
		{			
			//Serial.print("LED ring initializing ...");
			setEmbededPattern(0); // デフォルトパターンをセット
			ring_ = Adafruit_NeoPixel(18, 14, NEO_GRB+NEO_KHZ800);
			ring_.begin();
			ring_.setBrightness(255);
			clear();
			//Serial.println(" [ OK ]");
			return 0;
		}


		int8_t update(uint32_t frames) override
		{
			switch(mode_){
			case 0:
				{
					FrameBuffer foregroundFrameBuffer = backgroundFrameBuffer_;
					for(int i=0;i<18;i++){
						ring_.setPixelColor(i,
											ring_.Color(foregroundFrameBuffer.led_[i].r_,
														foregroundFrameBuffer.led_[i].g_,
														foregroundFrameBuffer.led_[i].b_));
					}
					ring_.show();
					if(frames % 50 == 0){
						backgroundFrameBuffer_.incr(); // 固定パターンインクリメントによる単純回転
					}
				}
				break;
			case 1:
				{
					// do nothing
				}
				break;
			case 8:
				{
					FrameBuffer foregroundFrameBuffer = backgroundFrameBuffer_;
					for(int i=0;i<18;i++){
						ring_.setPixelColor(i,
											ring_.Color(foregroundFrameBuffer.led_[i].r_,
														foregroundFrameBuffer.led_[i].g_,
														foregroundFrameBuffer.led_[i].b_));
					}
					ring_.show();
				}
				break;
			default:
				break;
			}
			return 0;
		}

		int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length) override
		{
			if(strcmp(type, "LEDR") == 0){
				switch(subtype){
				case 0:
					{
						// 組み込みパターン
						clear();
						setEmbededPattern(0);
						mode_ = 0;
					}
					break;
				case 1:
					{
						// ダイレクト制御（全消灯）
						clear();
						mode_ = 1;
					}
				case 2:					
					{
						// ダイレクト制御（指定点灯）
						clear();
						mode_ = 1;
						for(uint8_t i=0;i<length;i=i+4){
							uint8_t index = static_cast<uint8_t>(body[i]);
							uint8_t r = static_cast<uint8_t>(body[i+1]);
							uint8_t g = static_cast<uint8_t>(body[i+2]);
							uint8_t b = static_cast<uint8_t>(body[i+3]);
							Color c(r,g,b);
							on(index,c);
						}
					}
					break;
				case 8:
					{			
						// 外部制御モード（アニメーション用）
						mode_ = 8;
						FrameBuffer recv_fb;
						uint8_t index = 0;
						for(uint8_t i=0;i<length;i=i+3){
							uint8_t r = static_cast<uint8_t>(body[i]);
							uint8_t g = static_cast<uint8_t>(body[i+1]);
							uint8_t b = static_cast<uint8_t>(body[i+2]);
							recv_fb.led_[index++] = Color(r,g,b);
						}
						backgroundFrameBuffer_ = recv_fb;					
					}
					break;
				}
			}			
			return 0;
		}

private:

		/**
		   @brief 組み込みモード. 外部制御が基本であり最小限のパターンのみ組み込む.
		   @note ホスト起動時, 終了時等, 外部制御に頼れない場合のパターン等
		   @param [in] mode モード番号
		*/
		void setEmbededPattern(int mode)
		{
			mode_ = mode;
			if(mode == 0){
				// 起動時の表示モード
				FrameBuffer frame;
				double a = 0.4;
				double b = 0.6;
				frame.led_[4]  = Color(255,255,255); // center
				frame.led_[3]  = Color(255,255,255);
				frame.led_[2]  = Color(255*a*a,255*a*a,255*b);			
				frame.led_[1]  = Color(255*a*a*a,255*a*a*a,255*b*b);			
				frame.led_[0]  = Color(255*a*a*a*a,255*a*a*a*a,255*b*b*b);			
				backgroundFrameBuffer_ = frame;
			}
		}

		void on(int8_t index, const Color& c)
		{
			ring_.setPixelColor(index, ring_.Color(c.r_,c.g_,c.b_));
			ring_.show();
		}

		void clear()
		{
			for(int8_t i=0;i<18;++i){
				ring_.setPixelColor(i, ring_.Color(0,0,0));
				ring_.show();
			}
		}
		
		const bool auto_luminance_adaptation_;
		Adafruit_NeoPixel ring_;
		FrameBuffer backgroundFrameBuffer_; // バックグラウンドフレームバッファ
		int8_t mode_; // アニメーション動作モード
	};
	
}
