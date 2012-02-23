#ifndef Radio_h
#define Radio_h

#define BAUDRATE 9600

// Error codes
#define SUCCESS 0
#define REMOTE_READ_ERROR 1
#define LOCAL_READ_ERROR 2
#define RESPONSE_TIME_ERROR 3

#include <Arduino.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include <alloca.h>

class Radio {
	
	public:
		// Default constructor
		Radio();
		// Intialize, setting serial port
		void begin(HardwareSerial* serialPort);
		// Send string over radio link
		void send(String msg);
		// Send AT Command to local XBee
		void sendATCommand(uint8_t *cmd);
		// Receive data
		void receive();
	protected:
		// Pointer to serial port used
		HardwareSerial* serialPort;
		// XBee module object for use by XBee library
		XBee xbee;
		// Destination Address
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
		
		// AT Command objects
		AtCommandRequest atRequest;
		AtCommandResponse atResponse;
	private:
		// SS Debug
		uint8_t ssRX;
		uint8_t ssTX;
		SoftwareSerial *nss;
};

#endif
