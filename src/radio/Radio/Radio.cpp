#include "Radio.h"

Radio::Radio() {
	// Default serial port
	this->serialPort = &Serial;
}

void Radio::begin(HardwareSerial &serialPort) {
	// Initialize XBee
	this->xbee = XBee();
	// Set XBee serial port 
	this->xbee.setSerial(serialPort);
	// Set XBee Baud Rate
	this->xbee.begin(BAUDRATE);
}

void Radio::send(uint8_t *payload, uint8_t payloadLength) {
	xbee.reset();
	zbTx = ZBTxRequest(destAddr64, payload, payloadLength);
	xbee.send(zbTx);
}

void Radio::sendATCommand(uint8_t *cmd) {
	xbee.reset();
	AtCommandRequest atRequest = AtCommandRequest(cmd);
  	xbee.send(atRequest);
}
