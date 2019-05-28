namespace sonar{

	/**
	 * @struct ColorStruct
	 * @brief RGB 色定義
	 */
	typedef struct ColorStruct
	{
		ColorStruct(): r_(0), g_(0), b_(0) {}
		ColorStruct(uint8_t r, uint8_t g, uint8_t b) : r_(r), g_(g), b_(b) {}
		uint8_t r_;
		uint8_t g_;
		uint8_t b_;
	} Color;

	/**
	 * @struct FrameBufferStruct
	 * @brief １フレームの定義。LED リングには 18 個の LED が実装されており、18 個全ての色の定義がなされたデータ
	 */
	typedef struct FrameBufferStruct
	{
		FrameBufferStruct(){}
		/**
		 * @brief 色定義を LED 1 個分、時計回りに回転させる
		 */
		void incr()
		{
			Color c17 = led_[17];
			for(int i=17;i>0;i--){
				led_[i] = led_[i-1]; // LED17<-LED16, LED16<-LED15, ... LED1<-LED0, 
			}
			led_[0] = c17; // LED0<-LED17(backup)
		}

		void decr()
		{
			Color c0 = led_[0];
			for(int i=0;i<17;++i){
				led_[i] = led_[i+1]; // LED0<-LED1, LED1<-LED2, ... LED16<-LED17,
			}
			led_[17] = c0; // LED17<-LED0(backup)
		}

		Color led_[18];
	} FrameBuffer;
	
	/**
	 * @class LEDRing
	 * @brief LED リングを表現するクラス
	 * @see sonar::Module
	 */
	class LEDRing : public Module
	{
	public:

		/**
		 * @brief C'tor
		 * @param [in] 明るさを周辺明度に合わせて自動調整する（未実装機能）
		 * @todo 周辺明度自動調整機能の実装
		 */
		LEDRing(bool auto_luminance_adaptation = false) :
			Module("LEDRing"),
			auto_luminance_adaptation_(auto_luminance_adaptation),
			mode_(0)
		{}

		/**
		 * @brief 初期化関数
		 * @details LED 点灯デフォルトパターンを設定する
		 * @return 0 if success
		 */
		int8_t init() override
		{			
			ring_ = Adafruit_NeoPixel(18, 14, NEO_GRB+NEO_KHZ800);
			ring_.begin();
			ring_.setBrightness(255);
			clear();
			setEmbededPattern();
			return 0;
		}

		/**
		 * @brief アップデート関数
		 * @param [in] frames フレーム番号
		 * @return 0 if success
		 * @details mode_ に従って、アップデート処理を分岐する、分岐内容は実装を参照
		 */
		int8_t update(uint32_t frames) override
		{

			switch(mode_){
			case 0:
				{
					// do nothing
				}
				break;
			case 1:
				{
					// ダブルバッファリングでの組み込みパターンの点灯
					FrameBuffer foregroundFrameBuffer = backgroundFrameBuffer_;
					for(int i=0;i<18;i++){
						ring_.setPixelColor(i,
											ring_.Color(foregroundFrameBuffer.led_[i].r_,
														foregroundFrameBuffer.led_[i].g_,
														foregroundFrameBuffer.led_[i].b_));
					}
					ring_.show();
					// アニメーションは、内部カウントアップされるフレーム数によって内部的に継続される
					if(frames % 17 == 0){
						if(motion_ == 1){
							backgroundFrameBuffer_.incr(); // 固定パターンインクリメントによる単純回転
						}else if(motion_ == 2){
							backgroundFrameBuffer_.decr(); // 固定パターンデクリメントによる単純回転
						}
					}
				}
				break;
			case 8:
				{
					// 外部制御モード、外部指示に基づく点灯（すなわちアニメーションについても外部指示に基づく）
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

		/**
		 * @brief 通信（外部からの命令）の解釈
		 * @param [in] type 命令タイプ
		 * @param [in] subtype 命令サブタイプ
		 * @param [in] body 命令ボディ
		 * @param [in] length 命令ボディの長さ
		 * @return 0 if success
		 */
		int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length) override
		{
			if(strcmp(type, "LEDR") == 0){
				switch(subtype){
				case 0: // v1.0 で単一組み込みパターン、v1.1 から全消灯へ（外部仕様変更）
					{
						clear();
						mode_ = 0;
					}
					break;
				case 1: // v1.0 では利用していなかった、v1.1 から組み込みパターンへ
					{
						clear();
						mode_ = 1;
						motion_ = static_cast<uint8_t>(body[0]);
						FrameBuffer recv_fb;
						uint8_t index = 0;
						for(uint8_t i=1;i<length;i=i+3){
							uint8_t r = static_cast<uint8_t>(body[i]);
							uint8_t g = static_cast<uint8_t>(body[i+1]);
							uint8_t b = static_cast<uint8_t>(body[i+2]);
							recv_fb.led_[index++] = Color(r,g,b);
						}
						backgroundFrameBuffer_ = recv_fb;
					}
					break;
				case 8:
					{			
						// 外部制御モード（連続アニメーション用）
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
		void setEmbededPattern()
		{
			// 起動時の表示モード
			FrameBuffer frame;
			float a = 0.4;
			float b = 0.6;
			frame.led_[4]  = Color(255,255,255); // center
			frame.led_[3]  = Color(255,255,255);
			frame.led_[2]  = Color(255*a*a,255*a*a,255*b);
			frame.led_[1]  = Color(255*a*a*a,255*a*a*a,255*b*b);
			frame.led_[0]  = Color(255*a*a*a*a,255*a*a*a*a,255*b*b*b);
			backgroundFrameBuffer_ = frame;
			mode_ = 1; // 組み込みパターン点灯モード
			motion_ = 1; // 単純回転
		}

		void on(int8_t index, const Color& c)
		{
			ring_.setPixelColor(index, ring_.Color(c.r_,c.g_,c.b_));
			ring_.show();
		}

		/**
		 * @brief 全消灯
		 */
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
		int8_t mode_;   // アニメーション動作モード
		int8_t motion_; // mode_ = 1 のときのサブモード

	};
	
}
