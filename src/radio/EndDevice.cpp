#include "EndDevice.h"

EndDevice::EndDevice() {
	XBeeAddress64 destAddr64 = XBeeAddress64(0x0, 0x0);
}

void EndDevice::requestData() {
	// Request data from coordinator
	// Sends frame id of last received frame
	XBeeAddress64 coordAddr = XBeeAddress64(0x0, 0x0);
	uint8_t payload[] = {zbTx.getFrameId()};
	uint8_t payloadLength = sizeof(payload);
	zbTx = ZBTxRequest(coordAddr, payload, payloadLength);
	xbee.send(zbTx);
}

void EndDevice::findCoordinator() {
	// Scan for available PAN

	// ...

	// Send join request to coordinator
	XBeeAddress64 coordAddr = XBeeAddrses64(0x0, 0x0);
	uint8_t payload[] = {'K'};
	uint8_t payloadLength = sizeof(payload);
	zbTx = ZBTxRequest(coordAddr, payload, payloadLength);
	xbee.send(zbTx);	

	// Await response from coordinator
	xbee.readPacket(5000);
	if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
		xbee.getResponse().getZBRxResponse(zbRx);
		if(zbRx.getDataLength() > 0 && zbRx.getData()[0] == 'K') {
			// Joined Network!
			// Signal this somehow
		}
	}
}