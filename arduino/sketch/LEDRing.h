namespace sonar{

#define LED_MODE_WAIT (0)
#define LED_MODE_LOOP (1)
#define LED_MODE_ONES (8)
#define LED_MODE_SHOW (9)

#define LED_MOTION_ONCE    (0)
#define LED_MOTION_TYPE_1  (1)
#define LED_MOTION_TYPE_2  (2)
  
  
  typedef struct ColorStruct
  {
    uint8_t r_;
    uint8_t g_;
    uint8_t b_;

    ColorStruct(): r_(0), g_(0), b_(0) {}
    ColorStruct(uint8_t r, uint8_t g, uint8_t b) : r_(r), g_(g), b_(b) {}
  } Color;

  typedef struct FrameBufferStruct
  {
    Color led_[18];

    FrameBufferStruct(){}
    
    // LED_MOTION_TYPE_1
    void incr()
    {
      Color c17 = led_[17];
      for(int i=17;i>0;i--){
        led_[i] = led_[i-1]; // LED17<-LED16, LED16<-LED15, ... LED1<-LED0,
      }
      led_[0] = c17; // LED0<-LED17(backup)
    }
    
    // LED_MOTION_TYPE_2
    void decr()
    {
      Color c0 = led_[0];
      for(int i=0;i<17;++i){
        led_[i] = led_[i+1]; // LED0<-LED1, LED1<-LED2, ... LED16<-LED17,
      }
      led_[17] = c0; // LED17<-LED0(backup)
    }
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
     */
    LEDRing() :
      Module("LEDRing"),
      ring_(18,14, NEO_GRB + NEO_KHZ800),
      mode_(0)
    {}


    int8_t init() override
    {
      Serial.print("ledring initializing ...");
      mode_ = LED_MODE_LOOP;
      ring_ = Adafruit_NeoPixel(18, 14, NEO_GRB+NEO_KHZ800);
      ring_.begin();
      ring_.setBrightness(255);
      clear();
      setEmbededPattern(1,0,255,255,255); // デフォルトパターンをセット.

      led_timer = millis();
      Serial.println(" [ OK ]");
      return 0;
    }

    int8_t update(uint32_t frames) override
    {
      unsigned long now_timer = millis();
      switch(mode_){
      case LED_MODE_WAIT:
        {
        }
        break;
      case LED_MODE_LOOP:
        {
          FrameBuffer foregroundFrameBuffer = backgroundFrameBuffer_;
          for(int i=0;i<18;i++){
            ring_.setPixelColor(i,
                      ring_.Color(foregroundFrameBuffer.led_[i].r_,
                            foregroundFrameBuffer.led_[i].g_,
                            foregroundFrameBuffer.led_[i].b_));
          }
          
          if(motion_ == LED_MOTION_ONCE){
            // 停止.
            mode_ = LED_MODE_WAIT;
          }
          else if(motion_ == LED_MOTION_TYPE_1){
            // 固定パターンインクリメントによる回転.
            if(now_timer - led_timer > 50){
              backgroundFrameBuffer_.incr();
              led_timer = now_timer;
            }
          }
          else if(motion_ == LED_MOTION_TYPE_2){
            // 固定パターンデクリメントによる逆回転.
            if(now_timer - led_timer > 50){
              backgroundFrameBuffer_.decr();
              led_timer = now_timer;
            }
          }
          ring_.show();
        }
        break;
      case LED_MODE_ONES:
        {
          FrameBuffer foregroundFrameBuffer = backgroundFrameBuffer_;
          for(int i=0;i<18;i++){
            ring_.setPixelColor(i,
                    ring_.Color(foregroundFrameBuffer.led_[i].r_,
                          foregroundFrameBuffer.led_[i].g_,
                          foregroundFrameBuffer.led_[i].b_));
          }
          mode_ = LED_MODE_WAIT;
          ring_.show();
        }
        break;
      case LED_MODE_SHOW:
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
      int beforeMode = mode_;
      if(strcmp(type, "LEDR") == 0){
        switch(subtype){
        case 0:
          {
            // 組み込みパターン
            mode_ = LED_MODE_LOOP;
            clear();
            if(length >= 5){
             setEmbededPattern(body[0], body[1], body[2], body[3], body[4]);
            } else if(length > 1){
              setEmbededPattern(body[0], body[1], 255, 255, 255);
            } else if(length > 0){
              setEmbededPattern(body[0], 0, 255, 255, 255);
            } else {
              setEmbededPattern(1, 0, 255, 255, 255);
            }
          }
          break;
        case 1:
          {
            // ダイレクト制御（全消灯）
            mode_ = LED_MODE_WAIT;
            clear();
          }
          break;
        case 2:
          {
            // ダイレクト制御（全点灯）
            mode_ = LED_MODE_WAIT;
            for(uint8_t i=0;i<18;++i){
              setVal(
                static_cast<uint8_t>(i),
                static_cast<uint8_t>(body[0]),
                static_cast<uint8_t>(body[1]),
                static_cast<uint8_t>(body[2]));
            }
            ring_.show();
          }
          break;
        case 3:
          {
            // ダイレクト制御（指定点灯）
            mode_ = LED_MODE_WAIT;
            for(uint8_t i=0;i<length;i=i+4){
              setVal(
                static_cast<uint8_t>(body[i]),
                static_cast<uint8_t>(body[i+1]),
                static_cast<uint8_t>(body[i+2]),
                static_cast<uint8_t>(body[i+3]));
            }
            ring_.show();
          }
          break;
        case 8:
          {
            // 外部制御モード（アニメーション用）
            mode_ = LED_MODE_WAIT;
            uint8_t index = 0;
            for(uint8_t i=0;i<length;i=i+3){
              setVal(
                static_cast<uint8_t>(index),
                static_cast<uint8_t>(body[i]),
                static_cast<uint8_t>(body[i+1]),
                static_cast<uint8_t>(body[i+2]));
              index++;
            }
            for(; index < 18; ++index){
              setVal(
                static_cast<uint8_t>(index),
                static_cast<uint8_t>(0),
                static_cast<uint8_t>(0),
                static_cast<uint8_t>(0));
            }
            ring_.show();
/*
            mode_ = LED_MODE_ONES;
            motion_ = LED_MOTION_ONCE;
            FrameBuffer recv_fb;
            uint8_t index = 0;
            for(uint8_t i=0;i<length;i=i+3){
              recv_fb.led_[index++] = Color(
                       static_cast<uint8_t>(body[i]),
                       static_cast<uint8_t>(body[i+1]),
                       static_cast<uint8_t>(body[i+2]));
            }
            for(; index < 18; ++index){
              recv_fb.led_[index] = Color(0,0,0);
            }
            backgroundFrameBuffer_ = recv_fb;
*/
          }
          break;
        case 9:
          {
            // ボタン点灯モード.
            mode_ = 9;
            clear();
          }
          break;
        }

        if(beforeMode != mode_)
        {
          Serial.print("LED Change Mode: ");
          Serial.println(mode_);
        }
        else
        {
          // 連続してデータを受け取る可能性があるので応答は返さない。
          // Serial.println("LED Motion.");
        }
      }
      return 0;
    }

    /**
       @brief 組み込みモード. 外部制御が基本であり最小限のパターンのみ組み込む.
       @note ホスト起動時, 終了時等, 外部制御に頼れない場合のパターン等
       @param [in] motion モーション番号
       @param [in] colorVar カラーバリエーション
    */
    void setEmbededPattern(int8_t motion, int8_t colorVar, uint8_t r_, uint8_t g_, uint8_t b_)
    {
      // 0:固定、1:回転、2:逆回転
      motion_ = motion; 

      FrameBuffer frame;
      for(int i = 0; i < 18; ++i){
        frame.led_[i] = Color(0,0,0);
      }

      if(colorVar == 0){
        // 起動時の表示モード
        double a = 0.4;
        double b = 0.6;
        frame.led_[4]  = Color(r_,g_,b_); // center
        frame.led_[3]  = Color(r_,g_,b_);
        frame.led_[2]  = Color(r_*a*a,g_*a*a,b_*b);
        frame.led_[1]  = Color(r_*a*a*a,g_*a*a*a,b_*b*b);
        frame.led_[0]  = Color(r_*a*a*a*a,g_*a*a*a*a,b_*b*b*b);
      }
      else if(colorVar == 1){
        for(int i = 0; i < 18; i+=3){
        frame.led_[i]  = Color(r_,g_,b_);
        }
      }
      else if(colorVar == 2){
        for(int i = 0; i < 18; i+=6){
        frame.led_[i]  = Color(r_,g_,b_);
        }
      }
      
      backgroundFrameBuffer_ = frame;
    }

    // 引数：カラー
    void on(int8_t index, const Color& c)
    {
      ring_.setPixelColor(index, ring_.Color(c.r_,c.g_,c.b_));
    }
    
    // 引数：RGB
    void setVal(uint8_t index, uint8_t r_, uint8_t g_, uint8_t b_)
    {
      ring_.setPixelColor(index, ring_.Color(r_,g_,b_));
    }

    // フレーム色を直接編集する. 
    void setFrame(uint8_t index, uint8_t r_, uint8_t g_, uint8_t b_)
    {
      backgroundFrameBuffer_.led_[index] = Color(r_,g_,b_);
    }

    // 設定反映.
    void show()
    {
      ring_.show();
    }

    // 全消灯.
    void clear()
    {
      for(int8_t i=0;i<18;++i){
        ring_.setPixelColor(i, ring_.Color(0,0,0));
        backgroundFrameBuffer_.led_[i] = Color(0,0,0);
      }
      ring_.show();
    }

    // モード情報取得.
    int8_t getMode(){
      return mode_;
    }

private:
    Adafruit_NeoPixel ring_;
    FrameBuffer backgroundFrameBuffer_; // バックグラウンドフレームバッファ.
    int8_t mode_; // 点灯モード
    uint8_t motion_;// アニメーション動作モード.

    unsigned long led_timer = 0;
  };

}
