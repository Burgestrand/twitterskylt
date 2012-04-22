#ifndef Radio_h
#define Radio_h

#define BAUDRATE 9600

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <alloca.h>
#include <XBee.h>

class Radio {
	
	public:
		// Default constructor
		Radio();
		// Intialize, setting serial port
		void begin(HardwareSerial &serialPort);
		// Send data over radio link
		void send(uint8_t *payload, uint8_t payloadLength);
		// Send AT Command to local XBee
		void sendATCommand(uint8_t *cmd);
	protected:
		// Pointer to serial port used
		//HardwareSerial serialPort;
		// XBee module object for use by XBee library
		XBee xbee;
		// 64-bit destination address
		XBeeAddress64 destAddr64;
		// Reusable response and request objects
		// TX
		ZBTxRequest zbTx;
		ZBTxStatusResponse txStatus;
		// RX
		XBeeResponse response;
		ZBRxResponse zbRx;
		// Modem status response
		ModemStatusResponse msr;
		// AT Command response
		AtCommandResponse atResponse;
	private:
};

#endif
