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
		// Returns true iff an end device joins the network
		bool permitJoining(uint8_t seconds);
		// Check for data request from end device
		bool gotDataRequest();
	private:
};

#endif
