#include "Coordinator.h"

Coordinator::Coordinator() {

	// FSM not started
	state = CoordinatorNoStart;
	destAddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);

	// Initialize callback function pointers to default error handling
	callbackPt = NULL; 

	uint8_t initLength = 1;
	uint8_t initStr[] = {'I','N','I','T'};

	this->assoc = false;
	this->sending = false;
	this->currentPacket = 0;
}

uint8_t Coordinator::getState() {
	return state;
}

bool Coordinator::getAssoc() {
	return assoc;
}

String Coordinator::getStateName(uint8_t stateNum) {
	String retStr = "Not a state!";
	String stateStr[] = {"NoStart", "Start", "Init", "Network Formation Send",
	"Network Formation Receive", "Permit Joining Send", "Permit Joining Receive",
	"Await Join", "Join Response", "Join Response Delivery Report", "Idle",
	"Send Data", "Send Data Delivery Report", "Modem Status Action", "Error"};
	if(stateNum >= 0 && stateNum <= 14) {
		retStr = stateStr[stateNum];
	}
	return retStr;
}

void Coordinator::begin(HardwareSerial &serialPort) {
	// Initialize XBee
	this->xbee = XBee();
	// Set XBee serial port 
	this->xbee.setSerial(serialPort);
	// Set XBee Baud Rate
	this->xbee.begin(BAUDRATE);
	// Start FSM
	state = CoordinatorStart;
}

void Coordinator::pairUp() {
	// Reset network
	state = CoordinatorNetworkFormationSend;
}

void Coordinator::setData(uint8_t *data, uint8_t size) {
	const int maxPacketSize = 10;
	uint8_t fP = size/maxPacketSize;	// Number of full-length packets
	uint8_t rP = size%maxPacketSize;	// Size of remainder packet
	uint8_t tP = (rP>0 ? fP+1 : fP);	// Total number of packets

	// Fill up full-length packets
	for(int i=0; i<fP; i++) {
		for(int j=0; j<maxPacketSize; j++) {
			bufferPackets[i].data[j] = *(data+(i*maxPacketSize)+j);
		}
		bufferPackets[i].length = maxPacketSize;
	}

	// Fill up (potential) remainder packet
	if(tP != fP) {
		for(int j=0; j<rP; j++) {
			bufferPackets[fP].data[j] = *(data+(fP*maxPacketSize)+j);
		}
		bufferPackets[fP].data[rP] = '\0';
		bufferPackets[fP].length = rP+1;
	} 
}

void Coordinator::setErrorCallback(void (*callbackPt)(void)) {
	this->callbackPt = callbackPt;
}

void Coordinator::init() {
	this->timeOutFlag = false;
	this->timeOut = 0;
}

void Coordinator::startTimeOut(uint16_t timeoutTime) {
	// Set timeout
	timeOutFlag = true;
	timeOut = millis() + timeoutTime;
}

void Coordinator::checkTimeOut() {
	// Did we go over the time limit?
	if(millis() >= timeOut) {
		timeOutFlag = false;
		state = CoordinatorError;
	}
}

void Coordinator::sendAtCommand(uint8_t *cmd, CoordinatorState nextState) {
	sendATCommand(cmd);
	startTimeOut();
	state = nextState;
}

void Coordinator::awaitAtResponse(CoordinatorState nextState) {
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
			state = CoordinatorError;
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
		if(zbRx.getDataLength() == 1 && zbRx.getData()[0] == 'J') {
			// End device identified, give response
			state = CoordinatorJoinResponse;
		}
	}
}

void Coordinator::joinResponse() {
	uint8_t msg[] = {'K'};
	send(msg, sizeof(msg));
	startTimeOut();
	state = CoordinatorJoinResponseDelivery;
}

void Coordinator::dataDeliveryStatus() {
	checkTimeOut();
	// ZigBee Tx Response ('Delivery Report')
	if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {	
		timeOutFlag = false;
		xbee.getResponse().getZBTxStatusResponse(txStatus);
		// Get the delivery status (fifth byte of packet)
		if(txStatus.isSuccess()) {
			// Delivery Successful!
			SoftwareSerial nss = SoftwareSerial(9, 10);
			nss.begin(9600);
			nss.println("Delivery successful!");
			// Go back to (potentially) send next packet
			state = CoordinatorSendData;
		} 
		else {
			SoftwareSerial nss = SoftwareSerial(9, 10);
			nss.begin(9600);
			nss.println("Remote XBee did not receive packet - Failed delivery!");
			// The remote XBee did not receive our packet
 			state = CoordinatorError;
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
				delay(50);
				state = CoordinatorSendData;
			}
		}
		else if(xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			// Modem Status Response ('Notification from local XBee')
			state = CoordinatorModemStatusAction;
		
		}
		else if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE ||
				xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
			// ZigBee Tx Response ('Delivery Report')
			// Or AT Command Response
			// Something is wrong... should never get this type of package here!
			state = CoordinatorError;
		}
		else {
			// Unexpected Response type!
			state = CoordinatorError;
		}
	}
	else if (xbee.getResponse().isError()) {
		// Got an error!
		state = CoordinatorError;
	}
}

void Coordinator::sendData() {
	// Send data stored in private data field over XBee link
	// Data may be sent as multiple packets

	// Switch data and buffer pointers
	if(!sending) {
		for(int i=0; i<3; i++) {
			dataPackets[i] = bufferPackets[i];
		}
		currentPacket = 0;
	}

	if(dataPackets[currentPacket].length > 0) {
		// Send next packet
		sending = true;
		sendPacket(dataPackets[currentPacket]);
		currentPacket++;
	}
	else {
		// Done sending last packet
		sending = false;
		state = CoordinatorIdle;
	}
}

void Coordinator::sendPacket(packet dataPacket) {
	// Send a single packet over the XBee Link
	send(dataPacket.data, dataPacket.length);
	startTimeOut();
	state = CoordinatorSendDataDelivery;
}

void Coordinator::modemStatusAction() {
	// Got a Modem Status Response, let's find out what type
	xbee.getResponse().getModemStatusResponse(msr);

	if (msr.getStatus() == ASSOCIATED) {
		// Modem associated with network
		state = CoordinatorIdle;
	} 
	else if (msr.getStatus() == DISASSOCIATED) {
		// Modem disassociated (left network)
		// Something is wrong,  give error and wait for network reset
		state = CoordinatorError;
	} 
	else if (msr.getStatus() == COORDINATOR_STARTED) {
		// Coordinator has setup network
		state = CoordinatorIdle;
	}
	else if (msr.getStatus() == HARDWARE_RESET) {
		// Hardware reset signal
		state = CoordinatorStart;
	}
	else {
		// Something else (certainly not something we were expecting!)
		state = CoordinatorError;
	}	
}

void Coordinator::tick() {
	// Attempt to read new incoming packages (without timeout or block!)
	xbee.readPacket();

	uint8_t NwCmd[] = {'N', 'R', 0x0};
	uint8_t JoinCmd[] = {'N', 'J', PERMIT_JOIN_TIME};
	
	switch(state) {
		case CoordinatorNoStart:
			state = CoordinatorNoStart;
			break;
		case CoordinatorStart:
			// Move along, nothing to see here...
			state = CoordinatorInit;
			break;
		case CoordinatorInit:
			init();
			state = CoordinatorNetworkFormationSend;
			break;
		case CoordinatorNetworkFormationSend:
			sendAtCommand(NwCmd, CoordinatorNetworkFormationReceive);
			break;
		case CoordinatorNetworkFormationReceive:
			awaitAtResponse(CoordinatorPermitJoiningSend);
			break;
		case CoordinatorPermitJoiningSend:
			sendAtCommand(JoinCmd, CoordinatorPermitJoiningReceive);
			break;
		case CoordinatorPermitJoiningReceive:
			awaitAtResponse(CoordinatorAwaitJoin);
			startTimeOut(65000);
			break;
		case CoordinatorAwaitJoin:
			awaitJoin();
			break;
		case CoordinatorJoinResponse:
			assoc = true;
			joinResponse();
			break;
		case CoordinatorJoinResponseDelivery:
			dataDeliveryStatus();
			break;
		case CoordinatorIdle:
			idle();
			break;
		case CoordinatorSendData:
			sendData();
			break;
		case CoordinatorSendDataDelivery:
			dataDeliveryStatus();
			break;
		case CoordinatorModemStatusAction:
			modemStatusAction();
			break;
		case CoordinatorError:
			assoc = false;
			callbackPt();
			break;
		default:
			// Stay in current state (should never be here, really)
			break;
	}
}
