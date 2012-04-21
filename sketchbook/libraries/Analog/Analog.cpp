#include "Analog.h"

Analog::Analog() {
	buttonsDefined = false;
}

Analog::Analog(uint8_t buttonPin1, uint8_t buttonPin2) {	
  	this->btn1 = buttonPin1;
  	this->btn2 = buttonPin2;
  	buttonsDefined = true;
}

float Analog::voltageToTemperature(float voltage) {
	return (voltage-0.5)*100;
}

float Analog::getPinVoltage(uint8_t pin) {
	return (analogRead(pin) * V_RES);

}

float Analog::getBatteryVoltage(uint8_t pin) {
  // Voltage divider divides voltage by 2
  // Value from analog port has resolution of 49mV (5V/1024)
  // Also adjusts for voltage drop across MOSFET (approximation)
  int batteryLevel = analogRead(0);
  float Vbatt = (2*batteryLevel*V_RES) - 0.04*(batteryLevel/Q_DROP);
  return Vbatt;
}

bool Analog::buttonPressed(uint8_t button) {
	if(!buttonsDefined) {
		return false;
	}
	uint8_t btnPin = (button == 0 ? btn1 : btn2);
	float voltage = getPinVoltage(btnPin);
	if(voltage < V_TH) {
		return true;
	}
	return false;
}
