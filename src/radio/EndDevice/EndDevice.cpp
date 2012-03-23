#include "EndDevice.h"

// Working on:
// joinNetwork:
//   Startup: wait TIMEOUT for associated
//   Join:    issue NR AT-command, wait TIMEOUT for associated
//            go immediately to join state, ignoring everything else?

// TODO: tick reading packets and states possibly not consuming them <- merge/semi-merge states kinda!
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

// Access the twitter message buffer. Only call this after
// tick has returned TICK_NEW_MSG and tick has not been called again
// after that!
uint8_t *EndDevice::getData() {
	return data;
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
uint8_t EndDevice::tick() {
	xbee.readPacket();
	switch(State) {
		// XXX INIT: Temporary
		case EndDeviceInit:
			delay(1000);
			init();
			State = EndDeviceFormingNetwork;
			return TICK_OK;
		case EndDeviceStart:
			return start();
		case EndDeviceFormingNetwork:
			return formingNetwork();
		case EndDeviceJoiningSend:
			return joiningSend();
		case EndDeviceJoiningWait:
			return joiningWait();
		case EndDeviceJoiningWaitResponse:
			return joiningWaitResponse();
		case EndDeviceIdle:
			return idle();
		case EndDeviceSleepTell:
			return sleepTell();
		case EndDeviceSleepWait:
			return sleepWait();
		case EndDeviceSleeping:
			return sleeping();
		case EndDeviceError:
			return error();
		case EndDeviceRequestSend:
			return requestSend();
		case EndDeviceRequestStatus:
			return requestStatus();
		case EndDeviceRequestWait:
			return requestWait();
		default:
			{ DEBUG_MSG("Bad state!"); }
			return TICK_UNKNOWN_ERROR;
	}
}

// XXX INIT: Temporary state thing, resets the xbee
uint8_t EndDevice::init() {
	uint8_t cmd[] = {'N', 'R'};
	AtCommandRequest atcr(cmd);
	xbee.send(atcr);
}

// Initial state, waits for the "Hardware reset" modem status message
// TODO INIT: Remove pretty much, just wait for modem status associated with timeout
uint8_t EndDevice::start() {
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
	return TICK_OK;
}

// TODO: What should be done if this does not arrive?
// TODO INIT: Remove pretty much, just wait for modem status associated with timeout
// Waits for the "Associated" modem status message 
uint8_t EndDevice::formingNetwork() {
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
	return TICK_OK;
}

// TODO: Move this into the previous state, when the previous state has been implemented
// Send the join message to the coordinator to make it aware of us
// and verify that the coordinator is really a TWITTERSKYLT BASE STATION (tm).
uint8_t EndDevice::joiningSend() {
	DEBUG_MSG("[JOINING SEND]");
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_t payload[] = {'J'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	State = EndDeviceJoiningWait;
	return TICK_OK;
}

// Wait for the delivery status of the join message.
uint8_t EndDevice::joiningWait() {
	DEBUG_MSG("[JOINING WAIT]");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// Join message was delivered
				setTimeout(5000);
				State = EndDeviceJoiningWaitResponse;
			} else {
				// TODO Message not received
				return TICK_JOIN_NOT_DELIVERED;
			}
		} else {
			// TODO: Message of other type
		}
	}
	return TICK_OK;
}

// Wait for the response to the join message
uint8_t EndDevice::joiningWaitResponse() {
	DEBUG_MSG("[JOINING WAIT RESPONSE]");
	if (hasTimedOut()) {
		// Timed out waiting for a response to the join message
		disableTimeout();
		State = EndDeviceError;
		return TICK_JOIN_TIMEOUT;
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);
			if (zbrr.getDataLength() == 1 && zbrr.getData()[0] == 'K') {
				// Got a correct response
				disableTimeout();
				State = EndDeviceIdle;
				return TICK_JOIN_OK;
			} else {
				// TODO: Got wrong response, ignore it?
				return TICK_JOIN_BAD_MSG; // XXX: Not sure if this check/return value is needed really
			}
		} else {
			// TODO: Other type of message
		}
	}
	return TICK_OK;
}

// Radio is idle, either start checking for a new message or put the radio to sleep.
uint8_t EndDevice::idle() {
	DEBUG_MSG("[IDLE]");
	if (updateFlag) {
		updateFlag = false;
		State = EndDeviceRequestSend;
	} else {
		State = EndDeviceSleepTell;
	}
	return TICK_OK;
}

// Tell the radio to go to sleep with the SLEEP_RQ pin.
uint8_t EndDevice::sleepTell() {
	DEBUG_MSG("[SLEEP TELL]");
	digitalWrite(SLEEP_RQ_PIN, HIGH);
	State = EndDeviceSleepWait;
	wakeupFlag = false;
	return TICK_OK;
}

// Consume packets the radio emits until it falls asleep.
uint8_t EndDevice::sleepWait() {
	DEBUG_MSG("[SLEEP WAIT]");
	if (xbee.getResponse().isAvailable()) { // Consume waiting messages
		// TODO: Check message for bad things
	} else if (!digitalRead(SLEEP_STATUS_PIN)) { // Radio is sleeping
		State = EndDeviceSleeping;
		return TICK_SLEEPING;
	}
	return TICK_OK;
}

// Radio is sleeping, do nothing or wake it up if necessary.
uint8_t EndDevice::sleeping() {
	DEBUG_MSG("[SLEEPING]");
	if (wakeupFlag) {
		digitalWrite(SLEEP_RQ_PIN, LOW);
		State = EndDeviceIdle;
	}
	return TICK_SLEEPING;
}

// Error state.
uint8_t EndDevice::error() {
	DEBUG_MSG("[ERROR]");
	return TICK_UNKNOWN_ERROR; // TODO How should this be handled?
}

// Send a request for new messages to the base station
uint8_t EndDevice::requestSend() {
	DEBUG_MSG("[REQUEST SEND]");
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_t payload[] = {'R'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	State = EndDeviceRequestStatus;
	return TICK_OK;
}

// Wait for the delivery status of the update request.
uint8_t EndDevice::requestStatus() {
	DEBUG_MSG("[REQUEST STATUS]");
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// The request was delivered
				setTimeout(5000);
				State = EndDeviceRequestWait;
			} else {
				// The request was not delivered
				return TICK_UPDATE_NO_DELIVERY;
			}
		} else {
			// TODO: Message of other type
		}
	}
	return TICK_OK;
}

// Recieve data from the base station.
uint8_t EndDevice::requestWait() {
	DEBUG_MSG("[REQUEST WAIT]");
	if (hasTimedOut()) {
		// Timed out waiting for more data
		disableTimeout();

		// Discard the partially received data
		dataEnd = data;

		// Retry the update 4 times.
		timesTimeout++;
		if (timesTimeout > 3) {
			State = EndDeviceError;
			return TICK_UPDATE_TIMEOUT;
		} else {
			State = EndDeviceRequestSend;
		}
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// Has a data packet
			disableTimeout();
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);

			// Copy the data into the buffer
			memcpy(dataEnd, zbrr.getData(), zbrr.getDataLength());
			dataEnd += zbrr.getDataLength();

			// Test if this was the final data packet for this message
			if (zbrr.getData()[zbrr.getDataLength()-1] == 0) {
				State = EndDeviceIdle;
				dataEnd = data;
				return TICK_NEW_MSG;
			} else {
				setTimeout(5000);
			}
		} else {
			// TODO: Other type of message
		}
	}
	return TICK_OK;
}
