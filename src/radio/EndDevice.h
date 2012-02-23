#ifndef EndDevice_h
#define EndDevice_h

#include "Radio.h"

// ZigBee End Device
class EndDevice : public Radio {
	public:
		// Default constructor
		EndDevice();
		// Request new display data from Coordinator
		void requestData();
		// Find and join network
		void findCoordinator();
	private:
};

#endif
