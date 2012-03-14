#include "Coordinator.h"

Coordinator::Coordinator() {
	// Debug
	assoc = false;
	// FSM not started
	state = CoordinatorNoStart;
	destAddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);

	// Initialize callback function pointers to default error handling
	callbackPt = NULL; 

	// Allocate some memory for our pointers
	uint8_t initLength = 4;
	this->data = (uint8_t*)alloca(initLength*sizeof(uint8_t));
	
	// Stick some characters in there for good measure
	this->data[0] = 'A';
	this->data[1] = 'B';
	this->data[2] = 'C';
	this->data[3] = '\0';
	this->dataSize = 4;

	this->sending = false;
	this->currentPacket = 0;

	this->dataBuffer =  (uint8_t*)alloca(initLength*sizeof(*uint8_t));
	this->dataBufferSize =  (uint8_t*)alloca(initLength*sizeof(uint8_t));
	this->packetBufferSize =  (uint8_t*)alloca(initLength*sizeof(uint8_t));
}

uint8_t Coordinator::getState() {
	return state;
}

bool Coordinator::getAssoc() {
	return assoc;
}

String Coordinator::getStateName(uint8_t stateNum) {
	String retStr = "Not a state!";
	switch(stateNum) {
		case 0:
			retStr = "NoStart";
			break;
		case 1:
			retStr = "Start";
			break;
		case 2:
			retStr = "\n **************** \n COORDINATOR RESET \n **************** \n Init";
			break;
		case 3:
			retStr = "Network Formation Send";
			break;
		case 4:
			retStr = "Network Formation Receive";
			break;
		case 5:
			retStr = "Permit Joining Send";
			break;
		case 6:
			retStr = "Permit Joining Receive";
			break;
		case 7:
			retStr = "Await Join";
			break;
		case 8:
			retStr = "Join Response";
			break;
		case 9:
			retStr = "Join Response Delivery Report";
			break;
		case 10:
			retStr = "Idle";
			break;
		case 11:
			retStr = "Send Data";
			break;
		case 12:
			retStr = "Send Data Delivery Report";
			break;
		case 13:
			retStr = "Modem Status Action";
			break;
		case 14:
			retStr = "Error";
			break;
		default:
			break;
	}
	return retStr;
}

void Coordinator::begin(HardwareSerial &serialPort) {
	// Initialize XBee
	this->xbee = XBee();
	// Set serial Port
	//this->serialPort = serialPort;
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

void Coordinator::setData(uint8_t *data, uint8_t dataSize) {

	uint8_t fP = size/MAX_PACKET_SIZE;	// Number of full-length packets
	uint8_t rP = size%MAX_PACKET_SIZE;	// Size of remainder packet
	uint8_t tP = (rP>0 ? fP+1 : fP);	// Total number of packets
	char ** msgP = (char**)alloca(tP*sizeof(char*));
	uint8_t * msgLen = (uint8_t*)alloca(tP*sizeof(uint8_t));

	// Fill up full-length packets
	for(int i=0; i<fP; i++) {
		msgP[i] = (char*)alloca((MAX_PACKET_SIZE+1)*sizeof(char));
		strncpy(msgP[i], source+(sizeof(char)*i*MAX_PACKET_SIZE), MAX_PACKET_SIZE);
		msgP[i][MAX_PACKET_SIZE] = '\0';
		msgLen[i] = MAX_PACKET_SIZE+1;
	}

	// Fill up (potential) remainder packet
	if(tP != fP) {
		msgP[fP] = (char*)alloca((rP+1)*sizeof(char));
		strncpy(msgP[fP], source+(fP*MAX_PACKET_SIZE), rP);
		msgP[fP][rP] = '\0';
		msgLen[fP] = rP+1;
	} 

	this->dataBuffer = msgP;			// Data buffer (array of char arrays)
	this->packetBufferSize = msgLen;	// Array containing packet sizes
	this->dataBufferSize = tP;			// Number of packets
}

void Coordinator::setErrorCallback(void (*callbackPt)(void)) {
	this->callbackPt = callbackPt;
}

void Coordinator::init() {
	this->timeOutFlag = false;
	this->timeOut = 0;
}

void Coordinator::startTimeOut(uint16_t timeoutTime) {
	// Set timeout five seconds from now
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
		// Get the delivery status, the fifth byte
		//if (txStatus.getDeliveryStatus() == SUCCESS) {
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
		else if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			// ZigBee Tx Response ('Delivery Report')
			// Something is wrong... should never get this type of package here!
			state = CoordinatorError;
		}
		else if(xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
			// AT Command Response ('Confirmation of command from local XBee')
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

	// Move buffer pointer to data pointer
	if(!sending) {
		data = dataBuffer;
		dataSize = dataBufferSize;
		packetSize = packetBufferSize;
		currentPacket = 0;
	}

	if(currentPacket < dataSize) {
		// Send packet
		sending = true;
		sendPacket(currentPacket)
		currentPacket++;
	}
	else {
		// Done sending last packet
		sending = false;
		state = Idle;
	}
}

void Coordinator::sendPacket(uint8_t packet) {
	// Send a single packet over the XBee Link
	send(this->data[packet], this->packetSize[packet]);
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

void Coordinator::error() {
	// Something is awfully wrong! We should lay low for a little while and wait for a reset.
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
