#ifndef Coordinator_h
#define Coordinator_h

#include "Radio.h"

// ZigBee Network Coordinator
class Coordinator : public Radio {
	public:
		// Default Constructor
		Coordinator();
		// Send string to network (broadcast)
		//void send(String msg);
		// Forms new ZigBee network
		void formNetwork();
		// Permits joining the network for a given period of time
		void permitJoining(uint8_t seconds);
		// Broadcasts coordinator's presence on network
		// void broadcastPresence();
	private:
};

#endif
