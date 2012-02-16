#include "EndDevice.h"

void EndDevice::requestData() {
	// Send start character
	serialPort->print(SOMCHAR);
	// Send Message Sequence Number
	serialPort->print(seqNum);
	// Send EOM character
	serialPort->print(EOMCHAR);
}

void EndDevice::findPANCoordinator() {
	
}