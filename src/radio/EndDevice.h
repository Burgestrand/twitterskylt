#ifndef EndDevice_h
#define EndDevice_h

#include "Radio.h"

// ZigBee End Device
class EndDevice : public Radio {
	public:
		// Request new data from Coordinator
		void requestData();
		// Find and join network
		void findPANCoordinator();
	private:
};

#endif
