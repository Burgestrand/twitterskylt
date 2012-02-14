#ifndef Radio_h
#define Radio_h

#include <Xbee.h>
#include <NewSoftSerial.h>

// Error codes
# define unknownResponseFormat 3
# define errorResponse 4

#define ON HIGH
#define OFF LOW

 // Delay time for LEDs (in ms)
#define delayTime 500

// LED Pin definitions
#define statusLed 11
#define errorLed 12
#define dataLed 10

class Radio {
	
	public:
		 // Default constructor
		Radio();
		void displayError(uint8_t errorCode);
	private:
		uint8_t option;
		uint8_t data;
		bool dFlag;
		XBee xbee;
		XBeeResponse response;
		Rx16Response rx16;
		Rx64Response rx64 ;
};