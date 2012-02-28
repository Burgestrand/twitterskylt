#include "Coordinator.h"

Coordinator::Coordinator() {
	XBeeAddress64 destAddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
	uint16_t destAddr16 = 0xFFFF;
}

void tick() {
	// enum State {Start, Init, NetworkFormation, PermitJoining, AwaitJoin, Idle, ModemStatus, Send, Error};
	// enum State {Start, Init, NetworkFormationSend, NetworkFormationReceive, PermitJoiningSend, PermitJoiningReceive, AwaitJoin, Idle, ModemStatus, Send, Error};

	xbee.readPacket();

	switch(State) {
		
		case Start:
			State = Init;
		case Init:
			init();
		case NetworkFormationSend:
			uint8_t cmd[] = {'N', 'R', 0x0};
			sendAtCommand(NetworkFormationReceive);
		case NetworkFormationReceive:
			awaitAtResponse(PermitJoiningSend);
		case PermitJoiningSend:
			uint8_t cmd[] = {'N', 'J', PERMIT_JOIN_TIME};
			sendAtCommand(PermitJoiningReceive);
		case PermitJoiningReceive:
			awaitAtReponse(AwaitJoin);
			startTimeOut();
		case AwaitJoin:
			awaitJoin();
		case JoinResponse:
			joinResponse();
		case JoinResponseDelivery:
			joinResponseDelivery();
		case Idle:
			idle();
		case ModemStatus:
			
		case Send:

		case Error:

			State = Error;
		case ATResponse:

		default:
	}
	
}

void init() {
	
}

void startTimeOut() {
	// Set timeout five seconds from now
	timeOutFlag = true;
	timeOut = millis() + 5000;
}

void checkTimeOut() {
	if(millis() >= timeOut) {
		timeOutFlag = false;
		State = Error;
	}
}

void Coordinator::sendAtCommand(uint8_t *cmd, State nextState) {
	uint8_t joinCmd[] = {'N','J', PERMIT_JOIN_TIME};
	sendATCommand(joinCmd);
	startTimeOut();
	State = nextState;
}

void Coordinator::awaitAtResponse(State nextState) {
	// Timeout before any response was received - go to error state
	checkTimeOut();

	if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
		// Got a response from Local XBee
		timeOutFlag = false;
		xbee.getResponse().getAtCommandResponse(atResponse);
		if(atResponse.isOk()) {
			// Command was successful!
			State = nextState;
		}
		else {
			// Command failed - go to error state
			State = Error;
		}
	}	
}

void awaitJoin() {
	// No join requests within timeout time - go to error state
	checkTimeOut();
	if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
		// Got a response from Local XBee
		timeOutFlag = false;
		xbee.getResponse().getZBRxResponse(zbRx);
		if(zbRx.getDataLength() == 1 && zbRx.getData()[0] = 'J') {
			// End device identified, give response
			State = JoinResponse;
		}
	}
}

void joinResponse() {
	uint8_t msg[] = {'K'};
	send(msg, sizeof(msg));
	State = JoinResponseDelivery;
	startTimeOut();
}

void joinResponseDelivery() {
	checkTimeOut();
	// ZigBee Tx Response ('Delivery Report')
	if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {	
		timeOutFlag = false;
		xbee.getResponse().getZBTxStatusResponse(txStatus);
		// Get the delivery status, the fifth byte
		if (txStatus.getDeliveryStatus() == SUCCESS) {
			// Delivery Successful!
			State = Idle;
		} 
		else {
			// The remote XBee did not receive our packet.
 			State = Error;	
 		}
	}
}

void idle() {
	// The art of being idle!
}