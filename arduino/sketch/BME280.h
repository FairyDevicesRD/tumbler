namespace sonar
{
	class BME280 : public Module
	{
	public:
		BME280() :
			Module("BME280"),
			temperature_(0.),
			pressure_(0.),
			humidity_(0.)
		{}

		int8_t init() override
		{
			Serial.print("BME280 initializing ... ");
			bme280_.setMode(0x76, 4, 4, 4, 3, 5, 0, 0);
			bme280_.readTrim();
			Serial.println(" [ OK ]");			
			return 0;
		}
		
		int8_t update(uint32_t frames) override
		{
			/*
			if(frames % constants::fps_ == 0){ // 1 FPS
				bme280_.readData(&temperature_,&pressure_,&humidity_);
				Serial.print("BME280: T=");
				Serial.print(temperature_);
				Serial.print(";H=");
				Serial.print(humidity_);
				Serial.print(";P=");
				Serial.println(pressure_);
			}
			*/
			return 0;
		}
	private:
		SSCI_BME280 bme280_;		
		double temperature_;
		double pressure_;
		double humidity_;
	};	
}
