#include "Radio.h"

Radio::Radio(HardwareSerial* serialPort) {
	// Set XBee serial port 
	this->serialPort = serialPort;
	// Set baud rate
	(this->serialPort)->begin(baudrate);
}

void Radio::send(char *charPtr) {
		
}

char* Radio::receive() {
	
}