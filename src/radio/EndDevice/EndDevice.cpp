#include "EndDevice.h"

// TODO: Handle sleeping
// TODO: Handle long messages and message ID:s
// TODO: Handle errors


// Public ---------------------------------------------------------------------
EndDevice::EndDevice() : xbee(){
	timeOutFlag = false;
	updateFlag = false;
	timesTimeout = 0;
	State = EndDeviceInit;
	data[0] = 0;
	dataEnd = data;
}

// Set new message callback
void EndDevice::setMsg(void (*msgcb)(char *)) {
	msg_callback = msgcb;
}

// Set the status change callback
void EndDevice::setStatus(void (*statuscb)(uint8_t)) {
	status_callback = statuscb;
}

// TODO implement
void EndDevice::joinNetwork() {
}

// Request a update of the message
// A call to the new message callback will be done when new data has
// been received
void EndDevice::getNewestMessage() {
	// XXX: This seems wrong?
	if (State == EndDeviceIdle) {
		updateFlag = true;
	}
}

void EndDevice::begin(long baud) {
	xbee.begin(baud);
}

// Private --------------------------------------------------------------------
// Internal timer/Timeout handling --------------------------------------------
// Check if the timer has timed out
bool EndDevice::hasTimedOut() {
	if (timeOutFlag) {
		unsigned long time = millis();
		// Check if time is within timeOut and timeOut + some number to
		// handle when millis() overflows
		// Also check if the interval wraps around and handle that
		if (timeOut + 1000 < timeOut) { 
			return time > timeOut || time < timeOut + 1000;
		} else {
			return time > timeOut && time < timeOut + 1000;
		}
	}
	return false;
}

// Set the timer to time out in msecs milliseconds
void EndDevice::setTimeout(unsigned long msecs) {
	// This may overflow, which is ok.
	timeOut = millis() + msecs;
	timeOutFlag = true;
}

// Disable the timer
void EndDevice::disableTimeout() {
	timeOutFlag = false;
}

// Internal state handling ----------------------------------------------------
void EndDevice::tick() {
	xbee.readPacket();
	if (xbee.getResponse().isAvailable()) {
		DEBUG_MSG("Tick: got a packet\n");
	}
	if (xbee.getResponse().isError()) {
		DEBUG_MSG("Tick: got a bad packet\n");
	}
	switch(State) {
		case EndDeviceInit:
			delay(1000);
			init();
			State = EndDeviceFormingNetwork;
			break;
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
			error();
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

// XXX: Temporary state thing, resets the xbee
void EndDevice::init() {
	uint8_t cmd[] = {'N', 'R'};
	AtCommandRequest atcr(cmd);
	xbee.send(atcr);
}

// Initial state, waits for the "Hardware reset" modem status message
void EndDevice::start() {
	DEBUG_MSG("In state START\n");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			// Has modem message
			ModemStatusResponse msr;
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == HARDWARE_RESET) {
				DEBUG_MSG("Got HW Reset\n");
				State = EndDeviceFormingNetwork;
			} else {
				// TODO: Other modem status, should not happen?
				DEBUG_MSG("Other modem status\n");
			}
		} else {
			// TODO: Other message type
			DEBUG_MSG("Other message type\n");
		}
	}
}

// Waits for the "Associated" modem status message 
// TODO: What should be done if this does not arrive?
void EndDevice::formingNetwork() {
	DEBUG_MSG("In state FORMING NETWORK\n");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			ModemStatusResponse msr;
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == ASSOCIATED) {
				DEBUG_MSG("Got Associated\n");
				State = EndDeviceJoiningSend;
			} else {
				// TODO: Not associated meddelande
				DEBUG_MSG("Other modem status\n");
			}
		} else {
			// TODO: Annan meddelande-typ
			DEBUG_MSG("Other message type\n");
		}
	}
}

void EndDevice::joiningSend() {
	DEBUG_MSG("In state JOINING SEND\n");
	DEBUG_MSG("Sending join message\n");
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_t payload[] = {'J'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	State = EndDeviceJoiningWait;
}

void EndDevice::joiningWait() {
	DEBUG_MSG("In state JOINING WAIT\n");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// Status message of join message received 
				DEBUG_MSG("Got status of joining msg, awaiting response\n");
				setTimeout(5000);
				State = EndDeviceJoiningWaitResponse;
			} else {
				// TODO Message not received
				DEBUG_MSG("Joining message not received\n");
			}
		} else {
			// TODO: Message of other type
			DEBUG_MSG("Other message type\n");
		}
	}
}

void EndDevice::joiningWaitResponse() {
	DEBUG_MSG("In state JOINING WAIT RESPONSE\n");
	if (hasTimedOut()) {
		disableTimeout();
		DEBUG_MSG("Got no joining response\n");
		State = EndDeviceError;
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			DEBUG_MSG("Got a data response\n");
			// Got response
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);
			if (zbrr.getDataLength() == 1 && zbrr.getData()[0] == 'K') {
				DEBUG_MSG("Got correct join response\n");
				disableTimeout();
				State = EndDeviceIdle;
			} else {
				// TODO: Got wrong response
				DEBUG_MSG("Got wrong join response\n");
			}
		} else {
			// TODO: Other type of message
			DEBUG_MSG("Other message type\n");
		}
	}
}

void EndDevice::idle() {
	// TODO Have nothing to do, put radio to sleep
	DEBUG_MSG("In state IDLE\n");
	if (updateFlag) {
		updateFlag = false;
		DEBUG_MSG("Got update request from user\n");
		State = EndDeviceRequestSend;
	}
}

void EndDevice::error() {
	DEBUG_MSG("In state ERROR\n");
	// TODO: Report error.
}

void EndDevice::requestSend() {
	// Send message requesting data
	DEBUG_MSG("In state REQUEST SEND\n");
	DEBUG_MSG("Sending data request\n");
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_t payload[] = {'R'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	State = EndDeviceRequestStatus;
}

void EndDevice::requestStatus() {
	// Wait for status for request packet
	DEBUG_MSG("In state REQUEST STATUS\n");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// Status message of request message received 
				DEBUG_MSG("Got status of request msg, awaiting response\n");
				setTimeout(5000);
				State = EndDeviceRequestWait;
			} else {
				// TODO Request Message not received
				DEBUG_MSG("Data request message not received\n");
			}
		} else {
			// TODO: Message of other type
			DEBUG_MSG("Other message type\n");
		}
	}
}

void EndDevice::requestWait() {
	// Wait for response with data from coordinator
	DEBUG_MSG("In state REQUEST WAIT\n");
	if (hasTimedOut()) {
		disableTimeout();
		DEBUG_MSG("Timed out waiting for request data response\n");

		// Discard old data
		dataEnd = data;

		timesTimeout++;
		if (timesTimeout > 3) {
			State = EndDeviceError;
		} else {
			State = EndDeviceRequestSend;
		}
	} else if (xbee.getResponse().isAvailable()) {
		DEBUG_MSG("Got a message!\n");
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			disableTimeout();
			DEBUG_MSG("Got data response\n");
			// Got response
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);

			/*
			// TODO: Reformat debug code
			char len[3];
			DEBUG_MSG((char *) zbrr.getData());
			len[0] = ((char) zbrr.getDataLength()) + '0';
			len[1] = '\n';
			len[2] = '\0';
			DEBUG_MSG(len);
			
			for (int i=0; i < zbrr.getDataLength(); i++) {
				len[0] = zbrr.getData(i);
				DEBUG_MSG(len);
			}
			*/

			uint8_t *packetData = zbrr.getData();
			uint8_t len = zbrr.getDataLength();

			memcpy(dataEnd, packetData, len);
			dataEnd += len;
			if (packetData[len] == 0) {
				dataEnd = data;
				State = EndDeviceIdle;
				msg_callback((char *)data);
			} else {
				setTimeout(5000);
			}
		} else {
			// TODO: Other type of message
			DEBUG_MSG("Other message type\n");
		}
	}
}
