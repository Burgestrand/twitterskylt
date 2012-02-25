#include "Coordinator.h"

Coordinator::Coordinator() {
	XBeeAddress64 destAddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
}

void Coordinator::broadcast(String msg) {
	send(msg);
}

void Coordinator::formNetwork() {
	uint8_t idCmd[] = {'I','D', 0x3ABC};
	uint8_t wrCmd[] = {'W','R'};
	uint8_t resetCmd[] = {'N','R', 0x0};
	//receive();
	//sendATCommand(idCmd);
	//receive();
	sendATCommand(resetCmd);
	//receive();
	//sendATCommand(wrCmd);
}

void Coordinator::permitJoining(uint8_t seconds) {
	uint8_t joinCmd[] = {'N','J', seconds};
  	sendATCommand(joinCmd);
}
