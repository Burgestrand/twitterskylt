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

		// Puts modem into control mode, accepting commands
		void enterCtrlMode();
		// Enable/Disable low power mode ('sleep')
		void setSleepMode(bool sleep);

		// Listen on serial port
		String* receive();
		// Complete message available
		bool msgAvailable();
		// Read available characters
		void readAvailable();
		// Create new buffer
		void newBuffer();
	protected:
		// Sequence numbering for messages
		uint8_t seqNum;
		// Pointer to serial port used
		HardwareSerial* serialPort;
	private:
		// Input buffer
		String rxBuffer;
		// Flag indicating complete string read to buffer
		bool readComplete;
		// Flag indicating ŕadio reading message
		bool readInProgress;
};

#endif
