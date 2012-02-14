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
		// Flash error code on Error LED
		for (int i = 0; i < errorCode; i++) {
			digitalWrite(pin, HIGH);
			delay(delayTime);
			digitalWrite(pin, LOW);
		}
}

void Radio::Rx() {
    	xbee.readPacket();
    
   	 if (xbee.getResponse().isAvailable()) {
      
      		if (xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE) {

      				dFlag = 1;
        
       			if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
                			xbee.getResponse().getRx16Response(rx16);
        				option = rx16.getOption();
        				data = rx16.getData(0);
        			} 
        			else {
               			xbee.getResponse().getRx64Response(rx64);
        				option = rx64.getOption();
        				data = rx64.getData(0);
       			 }
     		 } 
     		 else {
        			displayError(unknownResponseFormat);    
     		}
    	} 
    	else if (xbee.getResponse().isError()) {
      		displayError(errorResponse);
	} 
}