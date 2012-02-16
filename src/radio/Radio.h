#ifndef Radio_h
#define Radio_h

// Character used for modem pair-up
#define JOINCHAR ('H')
// Start of Message Character
#define SOMCHAR ('\001')
// End of Message Character 
#define EOMCHAR ('\004')
// Baud rate for ZigBees
#define BAUDRATE 9600
// Size of rx buffer (bytes)
#define BUFFERSIZE 160

#include <Arduino.h>

class Radio {
	
	public:
		// Default constructor
		Radio();
		// Intialize, setting serial port
		void begin(HardwareSerial* serialPort);
		// End Device requests new data from Coordinator
		void requestData();
		// Used by End Device to find and join network
		void findPANCoordinator();
		// Used by Coordinator to listen for End Devices
		void permitJoining();
		// Puts modem into control mode, accepting commands
		void enterCtrlMode();
		// Enable/Disable low power mode ('sleep')
		void setSleepMode(bool sleep);
		// Send string via serial port
		void send(String msg);
		// Listen on serial port
		String* receive();
		// Complete message available
		bool msgAvailable();
		// Read available characters
		void readAvailable();
		// Create new buffer
		void newBuffer();
	private:
		// Input buffer
		String rxBuffer;
		// Pointer to serial port used
		HardwareSerial* serialPort;
		// Flag indicating complete string read to buffer
		bool readComplete;
		// Flag indicating ŕadio reading message
		bool readInProgress;
		// Sequence numbering for messages
		uint8_t seqNum;
};

#endif
