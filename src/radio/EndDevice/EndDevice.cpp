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

	pinMode(SLEEP_STATUS_PIN, INPUT);
	pinMode(SLEEP_RQ_PIN, OUTPUT);
	digitalWrite(SLEEP_RQ_PIN, LOW);
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
	/*if (State == EndDeviceIdle) {
		updateFlag = true;
	}
	*/
	// Is this better?
	updateFlag = true;
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
		case EndDeviceSleepTell:
			sleepTell();
			break;
		case EndDeviceSleepWait:
			sleepWait();
			break;
		case EndDeviceSleeping:
			sleeping();
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
			{ DEBUG_MSG("Bad state!"); }
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
				State = EndDeviceFormingNetwork;
			} else {
				// TODO: Other modem status, should not happen?
			}
		} else {
			// TODO: Other message type
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
				State = EndDeviceJoiningSend;
			} else {
				// TODO: Not associated meddelande
			}
		} else {
			// TODO: Annan meddelande-typ
		}
	}
}

void EndDevice::joiningSend() {
	DEBUG_MSG("[JOINING SEND]");
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
				setTimeout(5000);
				State = EndDeviceJoiningWaitResponse;
			} else {
				// TODO Message not received
			}
		} else {
			// TODO: Message of other type
		}
	}
}

void EndDevice::joiningWaitResponse() {
	DEBUG_MSG("[JOINING WAIT RESPONSE]");
	if (hasTimedOut()) {
		disableTimeout();
		State = EndDeviceError;
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// Got response
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);
			if (zbrr.getDataLength() == 1 && zbrr.getData()[0] == 'K') {
				disableTimeout();
				State = EndDeviceIdle;
			} else {
				// TODO: Got wrong response
			}
		} else {
			// TODO: Other type of message
		}
	}
}

void EndDevice::idle() {
	DEBUG_MSG("[IDLE]");
	if (updateFlag) {
		updateFlag = false;
		State = EndDeviceRequestSend;
	} else {
		State = EndDeviceSleepTell;
	}
}

void EndDevice::sleepTell() {
	DEBUG_MSG("[SLEEP TELL]");
	digitalWrite(SLEEP_RQ_PIN, HIGH);
	State = EndDeviceSleepWait;
	wakeupFlag = false;
}

void EndDevice::sleepWait() {
	DEBUG_MSG("[SLEEP WAIT]");
	if (xbee.getResponse().isAvailable()) { // Consume waiting messages
		// TODO: Check message for bad things
	} else if (!digitalRead(SLEEP_STATUS_PIN)) { // Radio is sleeping
		State = EndDeviceSleeping;
		status_callback(STATUS_SLEEPING);
	}
}

void EndDevice::sleeping() {
	DEBUG_MSG("[SLEEPING]");
	if (wakeupFlag) {
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
	DEBUG_MSG("[REQUEST SEND]");
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
				setTimeout(5000);
				State = EndDeviceRequestWait;
			} else {
				// TODO Request Message not received
			}
		} else {
			// TODO: Message of other type
		}
	}
}

void EndDevice::requestWait() {
	// Wait for response with data from coordinator
	DEBUG_MSG("[REQUEST WAIT]");
	if (hasTimedOut()) {
		disableTimeout();

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
				itoa(dataEnd - data, lenn, 10);
				dataEnd = data;
			} else {
				setTimeout(5000);
			}
		} else {
			// TODO: Other type of message
		}
	}
}
