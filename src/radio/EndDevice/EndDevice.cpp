#include "EndDevice.h"

// TODO: joinNetwork
// TODO: tick reading packets and states possibly not consuming them
// TODO: More robust long message handling?
// TODO: Handle errors better


// Public ---------------------------------------------------------------------
EndDevice::EndDevice() : xbee(){
	timeOutFlag = false;
	updateFlag = false;
	timesTimeout = 0;
	wakeupFlag = false;
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

// Tell the radio to wake up
void EndDevice::wakeup() {
	wakeupFlag = true;
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
		DEBUG_MSG("[Tick]: got a packet");
	}
	if (xbee.getResponse().isError()) {
		DEBUG_MSG("[Tick]: got a bad packet");
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
	DEBUG_MSG("[START]");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			// Has modem message
			ModemStatusResponse msr;
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == HARDWARE_RESET) {
				DEBUG_MSG("[START]: Got HW Reset");
				State = EndDeviceFormingNetwork;
			} else {
				// TODO: Other modem status, should not happen?
				DEBUG_MSG("[START]: Other modem status");
			}
		} else {
			// TODO: Other message type
			DEBUG_MSG("[START]: Other message type");
		}
	}
}

// Waits for the "Associated" modem status message 
// TODO: What should be done if this does not arrive?
void EndDevice::formingNetwork() {
	DEBUG_MSG("[FORMING NETWORK]");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			ModemStatusResponse msr;
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == ASSOCIATED) {
				DEBUG_MSG("[FORMING NETWORK]: Got Associated");
				State = EndDeviceJoiningSend;
			} else {
				// TODO: Not associated meddelande
				DEBUG_MSG("[FORMING NETWORK]: Other modem status");
			}
		} else {
			// TODO: Annan meddelande-typ
			DEBUG_MSG("[FORMING NETWORK]: Other message type");
		}
	}
}

void EndDevice::joiningSend() {
	DEBUG_MSG("[JOINING SEND]: Sending join message");
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_t payload[] = {'J'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	State = EndDeviceJoiningWait;
}

void EndDevice::joiningWait() {
	DEBUG_MSG("[JOINING WAIT]");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// Status message of join message received 
				DEBUG_MSG("[JOINING WAIT]: Got status of joining msg");
				setTimeout(5000);
				State = EndDeviceJoiningWaitResponse;
			} else {
				// TODO Message not received
				DEBUG_MSG("[JOINING WAIT]: Joining message not received");
			}
		} else {
			// TODO: Message of other type
			DEBUG_MSG("[JOINING WAIT]: Other message type");
		}
	}
}

void EndDevice::joiningWaitResponse() {
	DEBUG_MSG("[JOINING WAIT RESPONSE]");
	if (hasTimedOut()) {
		disableTimeout();
		DEBUG_MSG("[JOINING WAIT RESPONSE]: Timeout, got no joining response");
		State = EndDeviceError;
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// Got response
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);
			if (zbrr.getDataLength() == 1 && zbrr.getData()[0] == 'K') {
				DEBUG_MSG("[JOINING WAIT RESPONSE]: Got correct join response");
				disableTimeout();
				State = EndDeviceIdle;
			} else {
				// TODO: Got wrong response
				DEBUG_MSG("[JOINING WAIT RESPONSE]: Got wrong join response");
			}
		} else {
			// TODO: Other type of message
			DEBUG_MSG("[JOINING WAIT RESPONSE]: Other message type");
		}
	}
}

void EndDevice::idle() {
	// TODO Have nothing to do, put radio to sleep
	DEBUG_MSG("[IDLE]");
	if (updateFlag) {
		updateFlag = false;
		DEBUG_MSG("[IDLE]: Got update request from user");
		State = EndDeviceRequestSend;
	} else {
		DEBUG_MSG("[IDLE]: Putting radio to sleep");
		State = EndDeviceSleepTell;
	}
}

void EndDevice::sleepTell() {
	DEBUG_MSG("[SLEEP TELL]: Telling radio to go to sleep");
	digitalWrite(SLEEP_RQ_PIN, HIGH);
	State = EndDeviceSleepWait;
	wakeupFlag = false;
}

void EndDevice::sleepWait() {
	if (xbee.getResponse().isAvailable()) { // Consume waiting messages
		DEBUG_MSG("[SLEEP WAIT]: Consuming packet");
		// TODO: Check message for bad things
	} else if (!digitalRead(SLEEP_STATUS_PIN)) { // Radio is sleeping
		DEBUG_MSG("[SLEEP WAIT]: Radio is sleeping");
		State = EndDeviceSleeping;
		status_callback(STATUS_SLEEPING);
	}
}

void EndDevice::sleeping() {
	DEBUG_MSG("[SLEEPING]");
	if (wakeupFlag) {
		DEBUG_MSG("[SLEEPING]: Waking radio");
		digitalWrite(SLEEP_RQ_PIN, LOW);
		State = EndDeviceIdle;
	}
}

void EndDevice::error() {
	DEBUG_MSG("[ERROR]");
	status_callback(STATUS_ERROR);
}

void EndDevice::requestSend() {
	// Send message requesting data
	DEBUG_MSG("[REQUEST SEND]: Sending data request");
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_t payload[] = {'R'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	State = EndDeviceRequestStatus;
}

void EndDevice::requestStatus() {
	// Wait for status for request packet
	DEBUG_MSG("[REQUEST STATUS]");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// Status message of request message received 
				DEBUG_MSG("[REQUEST STATUS]: Got status of request msg");
				setTimeout(5000);
				State = EndDeviceRequestWait;
			} else {
				// TODO Request Message not received
				DEBUG_MSG("[REQUEST STATUS]: Data request message not received");
			}
		} else {
			// TODO: Message of other type
			DEBUG_MSG("[REQUEST STATUS]: Other message type");
		}
	}
}

void EndDevice::requestWait() {
	// Wait for response with data from coordinator
	DEBUG_MSG("[REQUEST WAIT]");
	if (hasTimedOut()) {
		disableTimeout();
		DEBUG_MSG("[REQUEST WAIT]: Timed out waiting for request data response");

		// Discard old data
		dataEnd = data;

		timesTimeout++;
		if (timesTimeout > 3) {
			State = EndDeviceError;
		} else {
			State = EndDeviceRequestSend;
		}
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			disableTimeout();
			// Got response
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);

			// TODO: Reformat debug code
			char lenn[10];
			//DEBUG_MSG((char *) zbrr.getData());
			itoa(zbrr.getDataLength(), lenn, 10);
			DEBUG_MSG("[REQUEST WAIT]: Package length:");
			DEBUG_MSG(lenn);
			
			/*
			for (int i=0; i < zbrr.getDataLength(); i++) {
				len[0] = zbrr.getData(i);
				DEBUG_MSG(len);
			}
			*/

			uint8_t *packetData = zbrr.getData();
			uint8_t len = zbrr.getDataLength();

			memcpy(dataEnd, packetData, len);
			dataEnd += len;
			if (packetData[len-1] == 0) {
				State = EndDeviceIdle;
				msg_callback((char *)data);
				DEBUG_MSG("[REQUEST WAIT]: Has complete message of length:");
				itoa(dataEnd - data, lenn, 10);
				DEBUG_MSG(lenn);
				dataEnd = data;
			} else {
				DEBUG_MSG("[REQUEST WAIT]: Has partial message");
				setTimeout(5000);
			}
		} else {
			// TODO: Other type of message
			DEBUG_MSG("[REQUEST WAIT]: Other message type");
		}
	}
}
