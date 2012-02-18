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
	// Set start time
	lastSendTime = millis();
	// Not in control mode on start
	inCtrlMode = false;
}

bool Radio::enterCtrlMode() {
	if(!inCtrlMode && (millis() - lastSendTime) > CTRLPADDING) {
		serialPort->print("+++");
		inCtrlMode = true;
		lastSendTime = millis();
	}
	if(inCtrlMode && (millis() - lastSendTime) > CTRLPADDING) {
		return true;
	}
	return false;
}

void Radio::sendCtrlCmd(String cmd) {
	String sCmd = "AT";
	sCmd += cmd;
	sCmd += "\r";
	serialPort->print(sCmd);
	lastSendTime = millis();
}

void Radio::exitCtrlMode() {
	serialPort->print("ATCN\r");
	lastSendTime = millis();
	inCtrlMode = false;	
}

bool Radio::isInCtrlMode() {
	return inCtrlMode;
}

char Radio::returnedOK () {
  // this function checks the response on the serial port to see if it was an "OK" or not
  char incomingChar[3];
  char okString[] = "OK";
  char result = 'n';
  int startTime = millis();
  while (millis() - startTime < 2000 && result == 'n') {  // use a timeout of 10 seconds
    if (Serial.available() > 1) {
      // read three incoming bytes which should be "O", "K", and a linefeed:
      for (int i=0; i<3; i++) {
        incomingChar[i] = Serial.read();
      }
      if ( strstr(incomingChar, okString) != NULL ) { // check to see if the respose is "OK"
//      if (incomingChar[0] == 'O' && incomingChar[1] == 'K') { // check to see if the first two characters are "OK"
        result = 'T'; // return T if "OK" was the response
      }  
      else {
        result = 'F'; // otherwise return F
      }
    }
  }
  return result;
}

void Radio::readResponse() {
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
