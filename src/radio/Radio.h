#ifndef Radio_h
#define Radio_h

// Character used for modem pair-up
#define joinCharacter ('H')
// Baud rate for ZigBees
#define baudrate 9600
// Size of rx buffer (bytes)
#define bufferSize 160

#include <Xbee.h>
#include <NewSoftSerial.h>

class Radio {
	
	public:
		// Default constructor
		Radio(HardwareSerial* serialPort);
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
		String receive();
		// Complete message available
		bool msgAvailable();
		// Read available characters
		void Radio::readAvailable();
		// Create new buffer
		void newBuffer();
	private:
		// Input buffer
		String rxBuffer;
		// Flag indicating complete string read to buffer
		bool readComplete;
		// Pointer to serial port used
		HardwareSerial* serialPort;
};
