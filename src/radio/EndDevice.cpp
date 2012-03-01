#include "EndDevice.h"


// TODO: Implement
bool EndDevicc::hasTimedOut() {
	return false;
}

// Public
EndDevice::EndDevice() : xbee(){
}

void EndDevice::joinNetwork() {
}

void EndDevice::getNewestMessage() {
}

void EndDevice::begin() {
	xbee.begin();
}

// Internal state handling
void EndDevice::tick() {

	xbee.getPacket();

	switch(State) {
		case EndDeviceStart:
			start();
			break;
		case EndDeviceFormingNetwork;
			break;
		case EndDeviceJoiningSend:
			break;
		case EndDeviceJoiningWait:
			break;
		case EndDeviceJoiningWaitResponse:
			break;
		case EndDeviceIdle:
			break;
		case EndDeviceError:
			break;
		case EndDeviceRequestSend:
			break;
		case EndDeviceRequestStatus:
			break;
		case EndDeviceRequestWait:
			break;
		default:
			// XXX: Should not happen
			break;
	}
}

void EndDevice::start() {
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			ModemStatusResponse msr();
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == HARDWARE_RESET()) {
				State = FormingNetwork;
			} else {
				// TODO: Other modem status, should not happen?
			}
		} else {
			// TODO: Other message type
	}
}

void EndDevice::formingNetwork() {
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			ModemStatusResponse msr();
			xbee.getResponse().getModemStatusResponse(msr);
			if (msr.getStatus() == ASSOCIATED) {
				State = JoiningSend;
			} else {
				// TODO: Not associated meddelande
			}
		} else {
			// TODO: Annan meddelande-typ
		}
	}
}

void EndDevice::joiningSend() {
	XBeeAddress64 addr64(0x0, 0x0);
	uint8_r payload[] = {'J'};
	ZBTxRequest zbtxr(addr64, payload, 1);
	xbee.send(zbtxr);
}

void EndDevice::joiningWait() {
	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			ZBTxStatusResponse zbtxsr;
			xbee.getResponse().getZBTxStatusResponse(zbtxsr);
			if (zbtxsr.isSuccess()) {
				// TODO Message received
				timeOutFlag = true;
				timeOut = millis() + 5000; // XXX Actual time for timeout?
				State = JoiningWaitResponse;
			} else {
				// TODO Message not received
			}
		} else {
			// TODO: Message of other type
		}
	}
}

void EndDevice::joiningWaitResponse() {
	if (hasTimedOut()) {
		// TODO: Got no response, but could send message?
	} else if (xbee.getResponse().isAvailable()) {
			if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// TODO: Got response
		} else {
			// TODO: Other type of message
		}
	} else {
		// TODO: Check for bad messages etc.
	}
}

void EndDevice::idle() {
	// TODO Have nothing to do, put radio to sleep
}

void EndDevice::error() {
	// TODO: Report error.
}

void EndDevice::requestSend() {
	// TODO: Send message requesting data
}

void EndDevice::requestStatus() {
	// TODO: Wait for status for request packet
}

void EndDevice::requestWait() {
	// TODO: Wait for response with data from coordinator
}
