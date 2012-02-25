#ifndef Coordinator_h
#define Coordinator_h

#include "Radio.h"

// ZigBee Network Coordinator
class Coordinator : public Radio {
	public:
		// Default Constructor
		Coordinator();
		// Send string to network (broadcast)
		void broadcast(String msg);
		// Forms a new ZigBee network, discarding old network configuration
		void formNetwork();
		// Permits joining the network for a given period of time (in seconds)
		void permitJoining(uint8_t seconds);
	private:
};

#endif
