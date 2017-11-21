namespace sonar
{
	/**
	   @class Module
	   @brief メインループから呼び出されるモジュールの基底クラス
	 **/
	class Module
	{
	public:
		/**
		   @brief コンストラクタ. 
		   @attention 初期化は init() 関数で行い、コンストラクタでは出来る限り何もしないこと.
		   @param [in] name 継承するモジュールの可読名称. 
		 **/
		Module(const char* name) : name_(name){}
		virtual ~Module(){}

		/**
		   @brief クラスの初期化(setup 関数から呼ばれる). オーバーライド必須.
		   @return 0 if success, otherwise fail.
		 **/
		virtual int8_t init() = 0;

		/**
		   @brief クラスのアップデートコード. アップデート間隔は呼び出し元で規定される. 
		   @attention シリアル通信のハンドラである Serial はグローバル変数として与えれているものを利用して良い. ただし出力側のみで、入力側の操作をしないこと.
		   @param [in] frames フレーム番号. 呼び出し元から与えられる. 
		   @return 0 if success, otherwise fail.
		**/
		virtual int8_t update(uint32_t frames){ return 0; }

		/**
		   @brief Raspberry Pi からのシリアル通信経由の命令受信. このクラスが受理できる命令が列挙される.
		   @details クラスが通信によって動作を変える場合にのみ使う. 通信によって外部から制御することができない場合は使わなくて良い.
		   @param [in] type タイプ(NUL終端文字列)
		   @param [in] subtype サブタイプ
		   @param [in] body データ本体(バイナリ)
		   @param [in] length データ本体の長さ
		   @return 0 if success, otherwise fail.
		 **/
		virtual int8_t recv(const char* type, uint8_t subtype, const char* body, uint8_t length){ return 0; }

		/**
		   @brief モジュール名称を返す. 管理・デバッグ用として用いる.
		   @return モジュール名称
		 **/
		const char* toString(){ return name_; }

	protected:
		const char* name_;
		
	};	
}
