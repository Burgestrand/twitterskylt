#include "EndDevice.h"

// TODO: Implement a lot of things
// Some helpers
// Error things


// TODO: Implement
bool EndDevice::hasTimedOut() {
	return false;
}

void EndDevice::debug(char *data) {
	if (debugActivated) {
		debug_callback(data);
	}
}

// Public
EndDevice::EndDevice() : xbee(){
	debugActivated = false;
	timeOutFlag = false;
	updateFlag = false;
	State = EndDeviceStart;
}

// Add debug callback and activate debugging
void EndDevice::setDebug(void (*dbgcb)(char *)) {
	debug_callback = dbgcb;
	debugActivated = true;
}

// Set new message callback
void EndDevice::setMsg(void (*msgcb)(char *)) {
	msg_callback = msgcb;
}

void EndDevice::joinNetwork() {
}

void EndDevice::getNewestMessage() {
	if (State == EndDeviceIdle) {
		updateFlag = true;
	}
}

//void EndDevice::begin(long baud) {
//	xbee.begin(baud);
//}

// Internal state handling
void EndDevice::tick() {

	xbee.readPacket();

	switch(State) {
		case EndDeviceStart:
			start();
			break;
		case EndDeviceFormingNetwork:
			formingNetwork();
			break;
		case EndDeviceJoiningSend:
			joiningSend();
			break;
		case EndDeviceJoiningWait:
			joiningWait();
			break;
		case EndDeviceJoiningWaitResponse:
			joiningWaitResponse();
			break;
		case EndDeviceIdle:
			idle();
			break;
		case EndDeviceError:
			break;
		case EndDeviceRequestSend:
			requestSend();
			break;
		case EndDeviceRequestStatus:
			requestStatus();
			break;
		case EndDeviceRequestWait:
			requestWait();
			break;
		default:
			// XXX: Should not happen
			break;
	}
}

// Initial state, waits for the "Hardware reset" modem status message
void EndDevice::start() {
	debug("In state START\n");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			// Has modem message
			ModemStatusResponse msr;
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == HARDWARE_RESET) {
				debug("Got HW Reset\n");
				State = EndDeviceFormingNetwork;
			} else {
				// TODO: Other modem status, should not happen?
				debug("Other modem status\n");
			}
		} else {
			// TODO: Other message type
			debug("Other message type\n");
		}
	}
}

// Waits for the "Associated" modem status message 
// TODO: What should be done if this does not arrive?
void EndDevice::formingNetwork() {
	debug("In state FORMING NETWORK\n");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			ModemStatusResponse msr;
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == ASSOCIATED) {
				debug("Got Associated\n");
				State = EndDeviceJoiningSend;
			} else {
				// TODO: Not associated meddelande
				debug("Other modem status\n");
			}
		} else {
			// TODO: Annan meddelande-typ
			debug("Other message type\n");
		}
	}
}

void EndDevice::joiningSend() {
	debug("In state JOINING SEND\n");
	debug("Sending join message\n");
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_t payload[] = {'J'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	State = EndDeviceJoiningWait;
}

void EndDevice::joiningWait() {
	debug("In state JOINING WAIT\n");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// Status message of join message received 
				debug("Got status of joining msg, awaiting response\n");
				timeOutFlag = true;
				timeOut = millis() + 5000; // XXX Actual time for timeout?
				State = EndDeviceJoiningWaitResponse;
			} else {
				// TODO Message not received
				debug("Joining message not received\n");
			}
		} else {
			// TODO: Message of other type
			debug("Other message type\n");
		}
	}
}

void EndDevice::joiningWaitResponse() {
	debug("In state JOINING WAIT RESPONSE\n");
	if (hasTimedOut()) {
		// TODO: Got no response, but could send message?
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// Got response
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);
			if (zbrr.getDataLength() == 1 && zbrr.getData()[0] == 'K') {
				debug("Got correct join response\n");
				timeOutFlag = false;
				State = EndDeviceIdle;
			} else {
				// TODO: Got wrong response
				debug("Got wrong join response\n");
			}
		} else {
			// TODO: Other type of message
			debug("Other message type\n");
		}
	}
}

void EndDevice::idle() {
	// TODO Have nothing to do, put radio to sleep
	debug("In state IDLE\n");
	if (updateFlag) {
		updateFlag = false;
		debug("Got update request from user\n");
		State = EndDeviceRequestSend;
	}
}

void EndDevice::error() {
	// TODO: Report error.
}

void EndDevice::requestSend() {
	// Send message requesting data
	debug("In state REQUEST SEND\n");
	debug("Sending data request\n");
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_t payload[] = {'R'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	State = EndDeviceRequestStatus;
}

void EndDevice::requestStatus() {
	// Wait for status for request packet
	debug("In state REQUEST STATUS\n");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// Status message of request message received 
				debug("Got status of request msg, awaiting response\n");
				timeOutFlag = true;
				timeOut = millis() + 5000; // XXX Actual time for timeout?
				State = EndDeviceRequestWait;
			} else {
				// TODO Request Message not received
				debug("Data request message not received\n");
			}
		} else {
			// TODO: Message of other type
			debug("Other message type\n");
		}
	}
}

void EndDevice::requestWait() {
	// Wait for response with data from coordinator
	debug("In state REQUEST WAIT\n");
	if (hasTimedOut()) {
		// TODO: Got no response, but could send the message?
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// Got response
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);

			char len[3];
			debug("Got data response\n");
			debug((char *) zbrr.getData());
			len[0] = ((char) zbrr.getDataLength()) + '0';
			len[1] = '\n';
			len[2] = '\0';
			debug(len);
			
			for (int i=0; i < zbrr.getDataLength(); i++) {
				len[0] = zbrr.getData(i);
				debug(len);
			}
			String str = "";
			msg_callback((char *) zbrr.getData());
			timeOutFlag = false;
			State = EndDeviceIdle;
		} else {
			// TODO: Other type of message
			debug("Other message type\n");
		}
	}
}
