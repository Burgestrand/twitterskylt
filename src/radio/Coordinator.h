#ifndef Coordinator_h
#define Coordinator_h

#include "Radio.h"

// ZigBee Network Coordinator
class Coordinator : public Radio {
	public:
		// Send string via serial port
		void send(String msg);
		// Used by Coordinator to listen for End Devices
		void permitJoining();
	private:
};

#endif
