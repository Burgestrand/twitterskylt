#include "Radio.h"

Radio::Radio() {
	// Default serial port
	this->serialPort = &Serial;
}

void Radio::begin(HardwareSerial &serialPort) {
	this->xbee = XBee();
	// Set XBee serial port 
	this->xbee.setSerial(serialPort);
	// Set XBee Baud Rate
	this->xbee.begin(BAUDRATE);

	// Debug over software serial 
	// Arduino pin 9 to Tx of USB-Serial device
	ssRX = 9;
	// Arduino pin 10 to Rx of USB-Serial device
	ssTX = 10;

	// Addresses
	XBeeAddress64 destAddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
	uint16_t destAddr16 = 0xFFFF;
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

	XBeeAddress64 destAddr64_n = XBeeAddress64(0x00000000, 0x0000FFFF);

	zbTx = ZBTxRequest(destAddr64_n, payload, payloadLength);
	txStatus = ZBTxStatusResponse();

	nss.println(" ");

	nss.println("Sending data...");

	xbee.send(zbTx);
	/*
	// After sending a tx request, we expect a status response

	// Wait up to half second for the status response
	xbee.readPacket(2000);

	if (xbee.getResponse().isAvailable()) {
	// Got a response!
	
  		nss.println("Got a response on sent data...");
  		// Should be a ZNet Tx status 
  		
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			xbee.getResponse().getZBTxStatusResponse(txStatus);

			nss.print("Got delivery status: ");
			nss.println(txStatus.getDeliveryStatus(), HEX);

			// Get the delivery status, the fifth byte
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
	*/
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
  	/*
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

        		uint8_t valueLength = atResponse.getValueLength();

        		if(valueLength > 0) {
        			nss.print("AT Command returned value [");
        		}
        		for(int i=0; i<atResponse.getValueLength(); i++) {
        			nss.print(atResponse.getValue()[i]);
        		}
        		if(valueLength > 0) {
        			nss.println("]");
        		}
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
	*/
}

struct ZBRxStruct Radio::receive() {
	
	struct ZBRxStruct response = {0,0};
	xbee.reset();
	SoftwareSerial nss(ssRX, ssTX);
	nss.begin(9600);
	nss.println(" ");
	nss.println("Reading incoming data...");

	// Read incoming data
	xbee.readPacket(2000);
	
	if (xbee.getResponse().isAvailable()) {
	
		// Got a package
		nss.println("Got a packet!");

		// Examine type of received package

		// ZigBee Rx Response ('Incoming data package from remote XBee')
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
	
			// Detected ZigBee packet
		    nss.println("Detected ZigBee packet");

			xbee.getResponse().getZBRxResponse(zbRx);

			response = (struct ZBRxStruct){zbRx.getDataLength(), zbRx.getData()};

			if (zbRx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
				// The sender got an ACK
				nss.println("ACK Successful!");
			}
			else {
				// Sender did not get an ACK
				nss.println("ACK Failed");
			}

		}
		// Modem Status Response ('Notification from local XBee')
		else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			xbee.getResponse().getModemStatusResponse(msr);

			response = (struct ZBRxStruct){0x1, (uint8_t*)0x0F};

			// Local XBee responding to certain events
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
		// ZigBee Tx Response ('Delivery Report')
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			xbee.getResponse().getZBTxStatusResponse(txStatus);

			response = (struct ZBRxStruct){0x1, (uint8_t*)0x10};

			nss.print("Got delivery status: ");
			nss.println(txStatus.getDeliveryStatus(), HEX);

			// Get the delivery status, the fifth byte
			if (txStatus.getDeliveryStatus() == SUCCESS) {
				// Success!
  				nss.println("Delivery was successful!");
			} 
			else {
				// The remote XBee did not receive our packet.
  				nss.println("Remote modem did not receive sent packet!");
			}
		}
		// AT Command Response ('Confirmation of command from local XBee')
		else if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {

			xbee.getResponse().getAtCommandResponse(atResponse);

			response = (struct ZBRxStruct){0x1, (uint8_t*)0x11};

			if (atResponse.isOk()) {
				// Command successful!
		        nss.print("Command [");
        		nss.print(atResponse.getCommand()[0]);
        		nss.print(atResponse.getCommand()[1]);
        		nss.println("] was successful!");

        		uint8_t valueLength = atResponse.getValueLength();

        		if(valueLength > 0) {
        			nss.print("AT Command returned value [");
        		}
        		for(int i=0; i<atResponse.getValueLength(); i++) {
        			nss.print(atResponse.getValue()[i]);
        		}
        		if(valueLength > 0) {
        			nss.println("]");
        		}
			} 
			else {
				// Command failed, got error code
		        nss.println("Received error code");
			}
		} 
		// Something else - Unexpected Response
		else {
			// Unexpected response   
			response = (struct ZBRxStruct){0x1, (uint8_t*)0x13};
			nss.println("Unexpected response from modem");
		}
	
	}
	// Error in response
	else if (xbee.getResponse().isError()) {
		// Got an error
		response = (struct ZBRxStruct){0x1, (uint8_t*)0x14};
		nss.println("Modem response error!");
	}

	return response;
}
