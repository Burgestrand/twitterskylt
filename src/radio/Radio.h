#ifndef Radio_h
#define Radio_h

// Character used for modem pair-up
#define joinCharacter ('H')
// Baud rate for ZigBees
#define baudrate 9600

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
		void send(char *charPtr);
		// Listen on serial port
		char* receive();
	private:
		char* rxBuffer;
		HardwareSerial* serialPort;
};