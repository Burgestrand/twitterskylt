#include "Coordinator.h"

Coordinator::Coordinator() {
	state = NoStart;
	super();
}

void Coordinator::begin() {
	state = Start;
}

void Coordinator::pairUp() {
	state = NetworkFormationSend;
}

void Coordinator::setData(uint8_t *data) {
	this->data = data;
}

uint8_t Coordinator::status() {
	if(status>3) {
		return 
	}
	// Status codes
	#define COORDINATOR_OK 0
	#define COORDINATOR_ERROR_NETWORK 1
	#define COORDINATOR_ERROR_AT_COMMAND 2
	#define COORDINATOR_ERROR_UNEXPECTED 3
}

void Coordinator::init() {
	
	uint8_t initLength = 5;
	this->data = (uint8_t*)alloca(initLength*sizeof(uint8_t));
	this->dataBuffer =  (uint8_t*)alloca(initLength*sizeof(uint8_t));

	this->status = 0;
	this->timeOutFlag = false;
	this->timeOut = 0;
}

void Coordinator::startTimeOut() {
	// Set timeout five seconds from now
	timeOutFlag = true;
	timeOut = millis() + TIMEOUT_TIME;
}

void Coordinator::checkTimeOut() {
	// Did we go over the time limit?
	if(millis() >= timeOut) {
		timeOutFlag = false;
		state = Error;
	}
}

void Coordinator::sendAtCommand(uint8_t *cmd, State nextState) {
	uint8_t joinCmd[] = {'N','J', PERMIT_JOIN_TIME};
	sendATCommand(joinCmd);
	startTimeOut();
	state = nextState;
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
			state = nextState;
		}
		else {
			// Command failed - go to error state
			state = Error;
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
			state = JoinResponse;
		}
	}
}

void Coordinator::joinResponse() {
	uint8_t msg[] = {'K'};
	send(msg, sizeof(msg));
	startTimeOut();
	state = JoinResponseDelivery;
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
			state = Idle;
		} 
		else {
			// The remote XBee did not receive our packet
 			state = Error;	
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
				state = SendData;
			}
		}
		else if(xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			// Modem Status Response ('Notification from local XBee')
			state = ModemStatusAction;
		
		}
		else if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			// ZigBee Tx Response ('Delivery Report')
			// Something is wrong... should never get this type of package here!
			state = Error;
		}
		else if(xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
			// AT Command Response ('Confirmation of command from local XBee')
			// Something is wrong... should never get this type of package here!
			state = Error;
		}
		else {
			// Unexpected Response type!
			state = Error;
		}
	}
	else if (xbee.getResponse().isError()) {
		// Got an error!
		state = Error;
	}
}

void Coordinator::sendData( ) {
	// Send data stored in private data field over XBee link
	send(this->data, sizeof(this->data));
	startTimeOut();
	state = SendDataDelivery;
}

void Coordinator::modemStatusAction() {
	// Got a Modem Status Response, let's find out what type
	xbee.getResponse().getModemStatusResponse(msr);

	if (msr.getStatus() == ASSOCIATED) {
		// Modem associated with network
		state = Idle;
	} 
	else if (msr.getStatus() == DISASSOCIATED) {
		// Modem disassociated (left network)
		// Something is wrong,  give error and wait for network reset
		state = Error;
	} 
	else if (msr.getStatus() == COORDINATOR_STARTED) {
		// Coordinator has setup network
		state = Idle;
	}
	else if (msr.getStatus() == HARDWARE_RESET) {
		// Hardware reset signal
		state = Start;
	}
	else {
		// Something else (certainly not something we were expecting!)
		state = Error;
	}	
}

void Coordinator::error() {
	
}

void Coordinator::tick() {

	// Attempt to read new incoming packages (without timeout or block!)
	xbee.readPacket();

	switch(State) {
		case NoStart:
			state = NoStart;
		case Start:
			// Move along, nothing to see here...
			state = Init;
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
