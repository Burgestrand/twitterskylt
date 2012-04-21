#ifndef Analog_h
#define Analog_h

#define V_RES 0.004882814
#define V_TH 0.7
#define Q_DROP 408

#include <Arduino.h>
#include <SoftwareSerial.h>

// Methods for analog input
class Analog {
	public:
		Analog();
		Analog(uint8_t buttonPin1, uint8_t buttonPin2);
		float voltageToTemperature(float voltage);
		float getPinVoltage(uint8_t pin);
		float getBatteryVoltage(uint8_t pin);
		bool buttonPressed(uint8_t button);
	private:
		uint8_t btn1;
		uint8_t btn2;
		bool buttonsDefined;
};

#endif
