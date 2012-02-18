#include "EndDevice.h"

// Red flag - code copying (Coordinator) ?
void EndDevice::requestData(String reqmsg) {
	// Send start character
	serialPort->print(SOMCHAR);
	// Send Message Sequence Number
	serialPort->print(reqmsg);
	// Send EOM character
	serialPort->print(EOMCHAR);
	// Update time for last send
	lastSendTime = millis();
}

void EndDevice::findPANCoordinator() {
	
}