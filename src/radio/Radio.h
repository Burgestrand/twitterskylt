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
// Time (in ms) padding for enter control mode command
#define CTRLPADDING 1200

#include <Arduino.h>

class Radio {
	
	public:
		// Default constructor
		Radio();
		// Intialize, setting serial port
		void begin(HardwareSerial* serialPort);

		/* Tries to modem into control mode, accepting commands.
		 * If successful, raises flag 'inControlMode'. 
		 * Take care to check this flag, and not to send anything
		 * for at least a second after entering control mode.
		 */ 
		bool enterCtrlMode();
		// Send control command
		void sendCtrlCmd(String cmd);
		// Exit control mode
		void exitCtrlMode();
		// Return in-control-mode flag
		bool isInCtrlMode();
		// Read control command response from XBee
		void readResponse();

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
		// Number of ms since last sent on serial port
		unsigned long lastSendTime;
		// Flag indicating XBee in control mode
		bool inCtrlMode;
	private:
		// Input buffer
		String rxBuffer;
		// Flag indicating complete string read to buffer
		bool readComplete;
		// Flag indicating ŕadio reading message
		bool readInProgress;
};

#endif
