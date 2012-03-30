#include "Coordinator.h"

Coordinator::Coordinator() {

	// FSM not started
	state = CoordinatorNoStart;
	destAddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
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
	String stateStr[] = {"NoStart", "Start", "Init", "NWF Tx",
	"NWF Rx", "PJ Tx", "PJ Rx",
	"Await Join", "Join Resp.", "JR Deliv.", "Idle",
	"Send Data", "Send Deliv.", "Modem", "Error", "Timeout"};
	if(stateNum >= 0 && stateNum <= 15) {
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
	const int maxPacketSize = 72;
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

			SoftwareSerial nss = SoftwareSerial(30, 32);
			nss.begin(9600);
			nss.print(tP);
			nss.print(" Packets: [");

			for(int i=0; i<tP; i++) {
				nss.print(bufferPackets[i].length);
				if(bufferPackets[i].length == 72)
				nss.print(", ");
			}

			nss.println("]");
}

uint8_t Coordinator::init() {
	this->timeOutFlag = false;
	this->timeOut = 0;
	this->assoc = false;
	state = CoordinatorNetworkFormationSend;
	return TICK_OK;
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
		state = CoordinatorTimeOut;
	}
}

uint8_t Coordinator::sendAtCommand(uint8_t *cmd, CoordinatorState nextState) {
	sendATCommand(cmd);
	startTimeOut();
	state = nextState;
	return TICK_OK;
}

uint8_t Coordinator::awaitAtResponse(CoordinatorState nextState) {
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
			// Command failed
			state = CoordinatorError;
			return TICK_AT_CMD_ERROR;
		}
	}
	return TICK_OK;
}

uint8_t Coordinator::awaitJoin() {
	// No join requests within timeout time - go to error state
	checkTimeOut();
	if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
		// Got a response from Remote XBee
		timeOutFlag = false;
		xbee.getResponse().getZBRxResponse(zbRx);
		if(zbRx.getDataLength() == 1 && zbRx.getData()[0] == 'J') {
			// End device identified, store remote address and give response
			destAddr64 = zbRx.getRemoteAddress64();

			SoftwareSerial nss = SoftwareSerial(30, 32);
			nss.begin(9600);
			nss.print("Saving remote address: [");
			nss.print(destAddr64.getMsb());
			nss.print("-");
			nss.print(destAddr64.getLsb());
			nss.println("]");
			state = CoordinatorJoinResponse;
		}
	}
	return TICK_OK;
}

uint8_t Coordinator::joinResponse() {
	uint8_t msg[] = {'K'};
	send(msg, sizeof(msg));
	startTimeOut();
	state = CoordinatorJoinResponseDelivery;
	this->assoc = true;
	return TICK_OK;
}

uint8_t Coordinator::dataDeliveryStatus() {
	checkTimeOut();
	// ZigBee Tx Response ('Delivery Report')
	if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {	
		timeOutFlag = false;
		xbee.getResponse().getZBTxStatusResponse(txStatus);
		// Get the delivery status (fifth byte of packet)
		if(txStatus.isSuccess()) {
			// Delivery Successful!
			if(state == CoordinatorJoinResponseDelivery) {
				state = CoordinatorIdle;
			}
			// Go back to (potentially) send next packet
			else {
				state = CoordinatorSendData;
			}
		} 
		else {
			// The remote XBee did not receive our packet
 			state = CoordinatorError;
 			return TICK_SEND_NO_DELIVERY;
 		}
	}
	return TICK_OK;
}

uint8_t Coordinator::idle() {
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
			// Something is wrong... should never get this type of packet here!
			return TICK_UNEXPECTED_PACKET;
		}
		else {
			// Unexpected Response type!
			return TICK_UNKNOWN_ERROR;
		}
	}
	else if (xbee.getResponse().isError()) {
		// Got an error!
		state = CoordinatorError;
		return TICK_UNKNOWN_ERROR;
	}
	return TICK_OK;
}

uint8_t Coordinator::sendData() {
	// Send data stored in private data field over XBee link
	// Data may be sent as multiple packets

	// Switch data and buffer pointers
	if(!sending) {
		for(int i=0; i<3; i++) {
			dataPackets[i] = bufferPackets[i];
		}
		currentPacket = 0;
	}

	if(dataPackets[currentPacket].length > 0 && currentPacket <= 3) {
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
	return TICK_OK;
}

uint8_t Coordinator::sendPacket(packet dataPacket) {
	// Send a single packet over the XBee Link
	send(dataPacket.data, dataPacket.length);
	startTimeOut();
	state = CoordinatorSendDataDelivery;
	return TICK_OK;
}

uint8_t Coordinator::modemStatusAction() {
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
		return TICK_MODEM_ERROR;
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
		return TICK_UNEXPECTED_MODEM_RESPONSE;
	}	
	return TICK_OK;
}

uint8_t Coordinator::error() {
	this->assoc = false;
	return TICK_ERROR;
}

uint8_t Coordinator::timeout() {
	this->assoc = false;
	return TICK_TIMEOUT;
}

uint8_t Coordinator::tick() {
	// Attempt to read new incoming packages (without timeout or block!)
	xbee.readPacket();

	uint8_t NwCmd[] = {'N', 'R', 0x0};
	uint8_t JoinCmd[] = {'N', 'J', PERMIT_JOIN_TIME};
	
	switch(state) {
		case CoordinatorNoStart:
			state = CoordinatorNoStart;
			return TICK_OK;
			break;
		case CoordinatorStart:
			// Move along, nothing to see here...
			state = CoordinatorInit;
			return TICK_OK;
			break;
		case CoordinatorInit:
			return init();
			break;
		case CoordinatorNetworkFormationSend:
			return sendAtCommand(NwCmd, CoordinatorNetworkFormationReceive);
			break;
		case CoordinatorNetworkFormationReceive:
			return awaitAtResponse(CoordinatorPermitJoiningSend);
			break;
		case CoordinatorPermitJoiningSend:
			return sendAtCommand(JoinCmd, CoordinatorPermitJoiningReceive);
			break;
		case CoordinatorPermitJoiningReceive:
			startTimeOut(65000);
			return awaitAtResponse(CoordinatorAwaitJoin);
			break;
		case CoordinatorAwaitJoin:
			return awaitJoin();
			break;
		case CoordinatorJoinResponse:
			return joinResponse();
			break;
		case CoordinatorJoinResponseDelivery:
			return dataDeliveryStatus();
			break;
		case CoordinatorIdle:
			return idle();
			break;
		case CoordinatorSendData:
			return sendData();
			break;
		case CoordinatorSendDataDelivery:
			return dataDeliveryStatus();
			break;
		case CoordinatorModemStatusAction:
			return modemStatusAction();
			break;
		case CoordinatorError:
			return error();
			break;
		case CoordinatorTimeOut:
			return timeout();
			break;
		default:
			return TICK_UNKNOWN_ERROR;
			// Stay in current state (should never be here, really)
			break;
	}
}
