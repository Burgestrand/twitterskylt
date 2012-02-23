#include "EndDevice.h"

EndDevice::EndDevice() {
	XBeeAddress64 destAddr64 = XBeeAddress64(0x0, 0x0);
}

void EndDevice::requestData() {
	// Request data
	send("RQ");
}

void EndDevice::findCoordinator() {
	
}