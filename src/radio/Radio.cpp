#include "Radio.h"

Radio::Radio(HardwareSerial* serialPort) {
	// Set XBee serial port 
	this->serialPort = serialPort;
	// Set baud rate
	(this->serialPort)->begin(baudrate);
	// Allocate rx buffer
	newBuffer();
}

void Radio::findPANCoordinator();

void Radio::permitJoining();

void Radio::enterCtrlMode();

void Radio::setSleepMode(bool sleep);

void Radio::send(String msg) {
	// Send string
	serialPort->print(msg);
	// Send stop sign (nullbyte)
	serialPort->write('\0');
}

String* Radio::receive() {
	return(readComplete ? &rxBuffer : NULL);
}

bool Radio::msgAvailable() {
	return readComplete;
}

void Radio::readAvailable() {
	while(serialPort->available() > 0) {
   		char inChar = (char)serialPort->read(); 
     	rxBuffer += inChar;
     	if (inChar == '\0') {
       		readComplete = true;
     	}
   	}
}

void Radio::newBuffer() {
	readComplete = false;
	rxBuffer = "";
	rxBuffer.reserve(bufferSize);
}
