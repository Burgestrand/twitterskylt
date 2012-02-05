/**
 * XBee Test
 */

#include "Radio.h"

Radio::Radio() {

	xbee = XBee();
	response = XBeeResponse();

	// Response holders
	rx16 = Rx16Response();
	rx64 = Rx64Response();

  	pinMode(statusLed, OUTPUT);
  	pinMode(errorLed, OUTPUT);
  	pinMode(dataLed,  OUTPUT);
  
  	// Initiate serial communication with radio module
	xbee.begin(9600);

	// Data carried by signal
	option = 0;
	data = 0;
	dFlag = 0;
}

bool Radio::dataAvailable() {
	return dFlag;
}

void Radio::displayError(int pin = errorLed, int errorCode) {

}

void Radio::rx() {
}