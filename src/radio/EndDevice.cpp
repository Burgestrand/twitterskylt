#include "EndDevice.h"

void EndDevice::requestData(String reqmsg) {
	// Send start character
	serialPort->print(SOMCHAR);
	// Send Message Sequence Number
	serialPort->print(reqmsg);
	// Send EOM character
	serialPort->print(EOMCHAR);
}

void EndDevice::findPANCoordinator() {
	
}