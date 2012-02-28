#include "Coordinator.h"

Coordinator::Coordinator() {
	XBeeAddress64 destAddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
	uint16_t destAddr16 = 0xFFFF;
}

void Coordinator::broadcast(String msg) {
	send(msg);
}

void Coordinator::formNetwork() {
	uint8_t idCmd[] = {'I','D', 0x129};
	uint8_t wrCmd[] = {'W','R'};
	uint8_t resetCmd[] = {'N','R', 0x0};
	//receive();
	//sendATCommand(idCmd);
	//receive();
	sendATCommand(resetCmd);
	//receive();
	//sendATCommand(wrCmd);
}

bool Coordinator::permitJoining(uint8_t seconds) {

	SoftwareSerial nss(ssRX, ssTX);
	nss.begin(9600);

	uint8_t joinCmd[] = {'N','J', seconds};
	sendATCommand(joinCmd);
	uint8_t a = 0;

	while(a<3) {
		
		xbee.readPacket(seconds*1000);
		
		if (xbee.getResponse().isAvailable() && xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			
			xbee.getResponse().getZBRxResponse(zbRx);
			if(zbRx.getDataLength() > 0 && zbRx.getData()[0] == 'J') {

				// Indentity confirmed, store address
				nss.print("Received End Device Join Request");
				this->destAddr64 = zbRx.getRemoteAddress64();
				this->destAddr16 = zbRx.getRemoteAddress16();
				uint8_t payload[] = {'K'};
				uint8_t payloadLength = sizeof(payload);
				zbTx = ZBTxRequest(zbRx.getRemoteAddress64(), payload, payloadLength);
				xbee.send(zbTx);	
				return true;
			}
		}		
		a++;
	}
	return false;
}

bool Coordinator::gotDataRequest() {
	
}