#include "Coordinator.h"

void Coordinator::send(String msg) {
	// Send start character
	serialPort->print(SOMCHAR);
	// Send message body
	serialPort->print(msg);
	// Send end character
	serialPort->print(EOMCHAR);
}

void Coordinator::permitJoining() {
	
}
