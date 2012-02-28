#include "Coordinator.h"

Coordinator::Coordinator() {
}

void Coordinator::begin() {
	
}

void Coordinator::pairUp() {
	
}

void Coordinator::setData(uint8_t *data) {
	
}

uint8_t Coordinator::status() {
	
}

void Coordinator::init() {
	
}

void Coordinator::startTimeOut() {
	// Set timeout five seconds from now
	timeOutFlag = true;
	timeOut = millis() + 5000;
}

void Coordinator::checkTimeOut() {
	// Did we go over the time limit?
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
			// The remote XBee did not receive our packet
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
			xbee.getResponse().getZBRxResponse(zbRx);
			if(zbRx.getDataLength() == 1 && zbRx.getData()[0] == 'R') {
				// Data request from End Device
				State = SendData;
			}
		}
		else if(xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			// Modem Status Response ('Notification from local XBee')
			State = ModemStatusAction;
		
		}
		else if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			// ZigBee Tx Response ('Delivery Report')
			// Something is wrong... should never get this type of package here!
			State = Error;
		}
		else if(xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
			// AT Command Response ('Confirmation of command from local XBee')
			// Something is wrong... should never get this type of package here!
			State = Error;
		}
		else {
			// Unexpected Response type!
			State = Error;
		}
	}
	else if (xbee.getResponse().isError()) {
		// Got an error!
		State = Error;
	}
}

void Coordinator::sendData( ) {
	// Send data stored in private data field over XBee link
	send(this->data, sizeof(this->data));
	startTimeOut();
	State = SendDataDelivery;
}

void Coordinator::modemStatusAction() {
	// Got a Modem Status Response, let's find out what type
	xbee.getResponse().getModemStatusResponse(msr);

	if (msr.getStatus() == ASSOCIATED) {
		// Modem associated with network
		State = Idle;
	} 
	else if (msr.getStatus() == DISASSOCIATED) {
		// Modem disassociated (left network)
		// Something is wrong,  give error and wait for network reset
		State = Error;
	} 
	else if (msr.getStatus() == COORDINATOR_STARTED) {
		// Coordinator has setup network
		State = Idle;
	}
	else if (msr.getStatus() == HARDWARE_RESET) {
		// Hardware reset signal
		State = Start;
	}
	else {
		// Something else (certainly not something we were expecting!)
		State = Error;
	}	
}

void Coordinator::error() {
	
}

void Coordinator::tick() {

	// Attempt to read new incoming packages (without timeout or block!)
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
		case ModemStatusAction:
			modemStatusAction();
		case Error:
			error();
		default:
			// Stay in current state (should never be here, really)
	}
}
