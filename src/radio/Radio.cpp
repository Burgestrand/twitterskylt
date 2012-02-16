#include "Radio.h"

Radio::Radio() {
	// Default serial port
	this->serialPort = &Serial;
}

void Radio::begin(HardwareSerial* serialPort) {
	// Set XBee serial port 
	this->serialPort = serialPort;
	// Set baud rate
	(this->serialPort)->begin(BAUDRATE);
	// Intiate sequence numbering
	seqNum = 0;
	// Allocate rx buffer
	newBuffer();
}

void Radio::enterCtrlMode() {
	serialPort->print("+++");
		
}

void Radio::setSleepMode(bool sleep) {
	
}

String* Radio::receive() {
	return(readComplete ? &rxBuffer : NULL);
}

bool Radio::msgAvailable() {
	return readComplete;
}

void Radio::readAvailable() {
	// Incoming data and buffer available
	while(serialPort->available() > 0 && readComplete == false) {
   		char inChar = (char)serialPort->read();
   		 // Valid start of message character received
   		 if(inChar == SOMCHAR) {
   		 	// If already reading message, discard old buffer and read new message
   		 	if(readInProgress == true) {
   		 		newBuffer();
   		 	}
   		 	readInProgress = true;
   		 }
	     else if (readInProgress) {
	     	// End of message character received, lock buffer
	    	if (inChar == EOMCHAR) {
	     		readComplete = true;
	     		readInProgress = false;
	     	}
   		 	// Read body of message
	     	else {
	     		rxBuffer += inChar;
	     	}
	     }
   	}
}

// Discard old buffer and prepare for next message
void Radio::newBuffer() {
	readComplete = false;
	readInProgress = false;
	rxBuffer = "";
	rxBuffer.reserve(BUFFERSIZE);
}
