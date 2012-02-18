#include "Coordinator.h"

// Red flag - code copying (EndDevice) ?
void Coordinator::send(String msg) {
	// Send start character
	serialPort->print(SOMCHAR);
	// Send message body
	serialPort->print(msg);
	// Send end character
	serialPort->print(EOMCHAR);
	// Update time for last send
	lastSendTime = millis();
}

void Coordinator::permitJoining() {
	
}
