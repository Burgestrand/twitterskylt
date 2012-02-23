#include "Radio.h"

Radio::Radio() {
	// Default serial port
	//this->serialPort = &Serial;
	//this->xbee = XBee();
}

void Radio::begin(HardwareSerial &serialPort) {
	this->xbee = XBee();
	// Set XBee serial port 
	this->xbee.setSerial(serialPort);
	// Set baud rate
	//(this->serialPort)->begin(BAUDRATE);
	// Set XBee Baud Rate
	this->xbee.begin(BAUDRATE);

	// Debug over serial 
	ssRX = 9;
	// Connect Arduino pin 10 to RX of usb-serial device
	ssTX = 10;
}

void Radio::send(String msg) {

	SoftwareSerial nss(ssRX, ssTX);
	nss.begin(9600);

	uint8_t payloadLength = msg.length();
	uint8_t length = payloadLength*sizeof(uint8_t);
	uint8_t *payload = (uint8_t*)alloca(length);

	for(int i=0; i<payloadLength; i++) {
		payload[i] = msg.charAt(i);
	}

	zbTx = ZBTxRequest(destAddr64, payload, payloadLength);
	txStatus = ZBTxStatusResponse();

	nss.println("Sending data...");

	xbee.send(zbTx);

	// After sending a tx request, we expect a status response,
	// wait up to half second for the status response
	xbee.readPacket(2000);

	if (xbee.getResponse().isAvailable()) {
	// Got a response!
	
  		nss.println("Got a response on sent data...");
  		// Should be a znet tx status 
  		
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			xbee.getResponse().getZBTxStatusResponse(txStatus);

			// get the delivery status, the fifth byte
			if (txStatus.getDeliveryStatus() == SUCCESS) {
				// Success!
  				nss.println("Delivery was successful!");
			} 
			else {
				// The remote XBee did not receive our packet.
  				nss.println("Remote modem did not receive sent packet!");
			}
		}
		else if (xbee.getResponse().isError()) {
			// Error reading package
			nss.println("Error reading response");
		} 
		else {
			// Local XBee did not provide a timely TX Status Response
			nss.println("Local XBee did not provide Tx response");
		}
	}
}

void Radio::sendATCommand(uint8_t *cmd) {

	xbee.reset();
	SoftwareSerial nss(ssRX, ssTX);
	nss.begin(9600);

	atResponse = AtCommandResponse();
	atRequest = AtCommandRequest(cmd);
  	
  	// Send command
  	nss.println("Sending AT Command...");
  	xbee.send(atRequest);
  	
  	// Wait a max of 5s for response
  	xbee.readPacket(2000);

	if (xbee.getResponse().isAvailable()) {

		// Should be an AT command response
		if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {

			xbee.getResponse().getAtCommandResponse(atResponse);

			if (atResponse.isOk()) {
				// Command successful!
		        nss.print("Command [");
        		nss.print(atResponse.getCommand()[0]);
        		nss.print(atResponse.getCommand()[1]);
        		nss.println("] was successful!");

			} 
			else {
				// Command failed, got error code
		        nss.println("Received error code");
			}
		} 
		else {
			// Got unexpected frame type
		    nss.println("Received unexpected frame type");
		}
	}
	
}

struct ZBRxStruct Radio::receive() {
	
	xbee.reset();
	SoftwareSerial nss(ssRX, ssTX);
	nss.begin(9600);

	nss.println("Reading incoming data...");
	// Read incoming data
	//xbee.readPacket(2000);
	if (xbee.getResponse().isAvailable()) {
		// Got a package

		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// Detected ZigBee packet
		    nss.println("Detected ZigBee packet");

			xbee.getResponse().getZBRxResponse(zbRx);

			if (zbRx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
				// The sender got an ACK
				nss.println("ACK Successful!");
			}
			else {
				// Sender did not get an ACK
				nss.println("ACK Failed");
			}

		}
		else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			xbee.getResponse().getModemStatusResponse(msr);
			// Local XBee responding to certain events (assoc and so on)
				if (msr.getStatus() == ASSOCIATED) {
					// Modem associated with network
					nss.println("Association Successful!");
				} 
				else if (msr.getStatus() == DISASSOCIATED) {
					// Modem disassociated (left network)
					nss.println("Disassociated");
				} 
				else if (msr.getStatus() == COORDINATOR_STARTED) {
					// Coordinator has setup network
					nss.println("Coordinator started!");
				}
				else if (msr.getStatus() == HARDWARE_RESET) {
					// Hardware reset signal
					nss.println("Hardware reset");
				}
				else {
					// Something else
					nss.print("Another command response received from modem: ");
					nss.println(msr.getStatus());
				}
		}
		else if (xbee.getResponse().isError()) {
			// Got and error
			nss.println("Modem response error!");
		}
		else {
			// Unexpected response   
			nss.println("Unexpected response from modem");
		}
	}
	
	//struct ZBRxStruct response = {0,0};
	struct ZBRxStruct response = {zbRx.getDataLength(), zbRx.getData()};
	return response;
}
