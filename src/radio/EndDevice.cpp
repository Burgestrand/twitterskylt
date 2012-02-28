#include "EndDevice.h"

/*
 * TODO
 * When to send stuff: Separate state or end of previous state?
 */

// Public
EndDevice::EndDevice() : xbee(){
}

void EndDevice::joinNetwork() {
}

void enddevice::getNewestMessage() {
}

void enddevice::begin() {
	xbee.begin();
}

// Internal state handling
void enddevice::tick() {

	xbee.getPacket();

	switch(State) {
		case Start:
			break;
		case FormingNetwork;
			break;
		case JoiningSend:
			break;
		case JoiningWait:
			break;
		case Idle:
			break;
		case Error:
			break;
		case RequestSend:
			break;
		case RequestWait:
			break;
		default:
			// XXX: Should not happen
			break;
	}
}

void enddevice::start() {
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			ModemStatusResponse msr();
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == HARDWARE_RESET()) {
				State = FormingNetwork;
			}
		}
	}
}

void enddevice::formingNetwork() {
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			ModemStatusResponse msr();
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == ASSOCIATED) {
				State = JoiningSend;
			}
		}
	}
}

void enddevice::joiningSend() {
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_r payload[] = {'J'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
	timeOutFlag = true;
	timeOut = millis() + 5000; // XXX Actual time for timeout?
}

void enddevice::joiningWait() {
	if (millis() >= timeOut) {
		// TODO Error
	} else if (xbee.getResponse().getApiId() == ZB_TX) {
	}

}

void enddevice::idle() {
}

void enddevice::error() {
}

void enddevice::requestSend() {
}

void enddevice::requestWait() {
}
