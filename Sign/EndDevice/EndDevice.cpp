#include "EndDevice.h"

// Working on:
// TODO: tick reading packets and states possibly not consuming them <- merge/semi-merge states kinda!
// TODO: More robust long message handling?
// TODO: Handle errors better


// Public ---------------------------------------------------------------------
EndDevice::EndDevice(uint8_t sleep_rq_pin, uint8_t sleep_status_pin) : xbee(){
	timeOutFlag = false;
	updateFlag = false;
	timesTimeout = 0;
	wakeupFlag = false;
	resetFlag = false;
	State = EndDeviceStart;
	data[0] = 0;
	dataEnd = data;

	this->sleep_rq_pin = sleep_rq_pin;
	this->sleep_status_pin = sleep_status_pin;

	pinMode(sleep_status_pin, INPUT);
	pinMode(sleep_rq_pin, OUTPUT);
	digitalWrite(sleep_rq_pin, LOW);
}

// Try to join a network, leaves any current network, aborts any current operation, wakes the radio
// if it is asleep.
// Note that a network join is attempted automatically at startup, so this function doesn't
// need to (shouldn't, even) be called when starting the arduino.
void EndDevice::joinNetwork() {
	if (State == EndDeviceSleepWait || State == EndDeviceSleeping) {
		resetFlag = true;
	} else {
		// resetStart() will handle all cleanup when switching to the restart state
		// so it is not necessary here.
		State = EndDeviceResetStart;
	}
}

// Request an update of the message
void EndDevice::getNewestMessage() {
	wakeup();
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

#define LED_1 A0
// Internal state handling ----------------------------------------------------
uint8_t EndDevice::tick() {
	xbee.readPacket();
	switch(State) {
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
		case EndDeviceResetStart:
			return resetStart();
		case EndDeviceResetWait:
			return resetWait();
		case EndDeviceWaking:
			return waking();
		default:
			{ DEBUG_MSG("Bad state!"); }
			return TICK_UNKNOWN_ERROR;
	}
}

// Send the reset command to the radio, ignoring any ongoing activities
uint8_t EndDevice::resetStart() {
	DEBUG_MSG("[RESET START]");

	// Reset everything
	resetFlag = false;
	disableTimeout();
	dataEnd = data;

	// Issue reset command
	uint8_t cmd[] = {'N', 'R'};
	AtCommandRequest atcr(cmd);
	xbee.send(atcr);

	State = EndDeviceResetWait;
	return TICK_OK;
}

// Wait for the response to the reset command, ignore everything else
uint8_t EndDevice::resetWait() {
	DEBUG_MSG("[RESET WAIT]");
	if (  xbee.getResponse().isAvailable()
	   && xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE ) {
		AtCommandResponse atcr;
		xbee.getResponse().getAtCommandResponse(atcr);
		if (atcr.getCommand()[0] == 'N' && atcr.getCommand()[1] == 'R') {
			State = EndDeviceStart;
		}
	}
	return TICK_OK;
}

// Set timeout for associaton to complete
uint8_t EndDevice::start() {
	DEBUG_MSG("[START]");
	updateFlag = false; // Make sure this flag is reset as it may stay active if radio is reset while the flag is active
	disableTimeout(); // Make sure the timer is not active afte a reset
	wakeupFlag = false;
	setTimeout(10000);
	State = EndDeviceFormingNetwork;
	return TICK_OK;
}

// Waits for the "Associated" modem status message 
uint8_t EndDevice::formingNetwork() {
	DEBUG_MSG("[FORMING NETWORK]");
	if (hasTimedOut()) {
		DEBUG_MSG("TIMEOUT");
		State = EndDeviceError;
		return TICK_ASSOC_FAIL;
		// TODO: Check association status and return appropriate error message.
	}
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			ModemStatusResponse msr;
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == ASSOCIATED) {
				disableTimeout();
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
				State = EndDeviceError;
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
				State = EndDeviceError;
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
		DEBUG_MSG("Going to sleep");
		digitalWrite(sleep_rq_pin, HIGH);
		State = EndDeviceSleepWait;
		wakeupFlag = false;
	}
	return TICK_OK;
}

// Consume packets the radio emits until it falls asleep.
uint8_t EndDevice::sleepWait() {
	DEBUG_MSG("[SLEEP WAIT]");
	if (xbee.getResponse().isAvailable()) { // Consume waiting messages
		// TODO: Check message for bad things
	} else if (!digitalRead(sleep_status_pin)) { // Radio is sleeping
		State = EndDeviceSleeping;
		return TICK_SLEEPING;
	}
	return TICK_OK;
}

// Radio is sleeping, do nothing or wake it up if necessary.
uint8_t EndDevice::sleeping() {
	DEBUG_MSG("[SLEEPING]");
	if (!(resetFlag || wakeupFlag)) {
		return TICK_SLEEPING;
	} else {
		digitalWrite(sleep_rq_pin, LOW);
	}

	State = EndDeviceWaking;
	return TICK_SLEEPING;
}

// Radio has been requested to awake, wait for that to happen
// Then go to idle/reset state depending on wake up reason.
uint8_t EndDevice::waking() {
	DEBUG_MSG("[WAKING]");
	if (digitalRead(sleep_status_pin)) {
		if (resetFlag) {
			State = EndDeviceResetStart;
		} else { // Must be the wakeupFlag that caused the awakening
			State = EndDeviceIdle;
		}
		return TICK_OK;
	} else {
		return TICK_SLEEPING;
	}
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
				State = EndDeviceError; // TODO? Retries?
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
			timesTimeout = 0;
			return TICK_UPDATE_TIMEOUT;
		} else {
			State = EndDeviceRequestSend;
		}
	} else if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			DEBUG_MSG("  GOT PACKET");
			// Has a data packet
			disableTimeout();
			ZBRxResponse zbrr;
			xbee.getResponse().getZBRxResponse(zbrr);

			// Copy the data into the buffer
			memcpy(dataEnd, zbrr.getData(), zbrr.getDataLength());
			dataEnd += zbrr.getDataLength();

			// Test if this was the final data packet for this message
			if (zbrr.getData()[zbrr.getDataLength()-1] == 0) {
				timesTimeout = 0; // Reset timeout counter as we got a complete message
				DEBUG_MSG("  GOT MSG");
				DEBUG_MSG((char *) data);
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
