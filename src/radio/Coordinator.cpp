#include "Coordinator.h"

Coordinator::Coordinator() {
}

void Coordinator::tick() {

	// Attempt to read new incoming packages (without timeout!)
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
			dataDeliveryStatus();
		case Idle:
			idle();
		case SendData:
			sendData();
		case SendDataDelivery:
			dataDeliveryStatus();
		case ModemStatus:
			
		case Send:

		case Error:

			State = Error;
		case ATResponse:

		default:
	}
	
}

void Coordinator::init() {
	
}

void Coordinator::startTimeOut() {
	// Set timeout five seconds from now
	timeOutFlag = true;
	timeOut = millis() + 5000;
}

void Coordinator::checkTimeOut() {
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

void Coordinator::awaitJoin() {
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

void Coordinator::joinResponse() {
	uint8_t msg[] = {'K'};
	send(msg, sizeof(msg));
	startTimeOut();
	State = JoinResponseDelivery;
}

void Coordinator::dataDeliveryStatus() {
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

void Coordinator::idle() {
	// Idle state. Coordinator is listening for all possible types of incoming packages
	if(xbee.getResponse().isAvailable()) {
		// Got a package! Wonder what type it is...?
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// ZigBee Rx Response ('Incoming data package from remote XBee')
		
		}
		else if(xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			// Modem Status Response ('Notification from local XBee')
		
		}
		else if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			// ZigBee Tx Response ('Delivery Report')
			
		}
		else if(xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
			// AT Command Response ('Confirmation of command from local XBee')
			
		}
		else {
			// Unexpected Response type!

		}
	}
	else if (xbee.getResponse().isError()) {
		// Got an error!
		State = Error;
	}
}

void Coordinator::sendData() {
	// Send data stored in private data field over XBee link
	send(this->data, sizeof(this->data));
	startTimeOut();
	State = SendDataDelivery;
}

void Coordinator::modemStatusAction() {
	
}