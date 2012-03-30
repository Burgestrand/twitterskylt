// DHCP Library v0.3 - April 25, 2009
// Author: Jordan Terrell - blog.jordanterrell.com

#include "w5100.h"

#include <string.h>
#include <stdlib.h>
#include "Dhcp.h"
#include "Arduino.h"
#include "util.h"

int DhcpClass::beginWithDHCP(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
    _dhcp_state = STATE_DHCP_START;
    _timeout = timeout;
    _responseTimeout = responseTimeout;
    
  
    // zero out _dhcpMacAddr, _dhcpSubnetMask, _dhcpGatewayIp, _dhcpLocalIp, _dhcpDhcpServerIp, _dhcpDnsServerIp
    memset(_dhcpMacAddr, 0, 26); 

    memcpy((void*)_dhcpMacAddr, (void*)mac, 6);
  
    // Pick an initial transaction ID
    _dhcpTransactionId = random(1UL, 2000UL);
    _dhcpInitialTransactionId = _dhcpTransactionId;
    
    return requestLease();
}

int DhcpClass::requestLease() {

    if (_dhcpUdpSocket.begin(DHCP_CLIENT_PORT) == 0)
    {
      // couldn't get a socket
      return 1;
    }
    
    presend_DHCP();
    
    int result = 1;
    uint8_t messageType = 0;
    uint8_t prevState;
    
    unsigned long startTime = millis();
    unsigned long randomDiff;
    unsigned long responseTimeout = _responseTimeout;

    while(_dhcp_state != STATE_DHCP_LEASED)
    {
      
      randomDiff = (unsigned long)random(0,3)*1000;
        if(_dhcp_state == STATE_DHCP_START)
        {
            _dhcpTransactionId++;
            W5100.setIPAddress(IPAddress(0,0,0,0).raw_address());
            send_DHCP_MESSAGE(DHCP_DISCOVER, ((millis() - startTime) / 1000));
	    prevState = _dhcp_state;
            _dhcp_state = STATE_DHCP_DISCOVER;
        }
        else if(_dhcp_state == STATE_DHCP_DISCOVER)
        {
            uint32_t respId;
            messageType = parseDHCPResponse(responseTimeout+randomDiff-1000, respId);
            if(messageType == DHCP_OFFER)
            {
                // We'll use the transaction ID that the offer came with,
                // rather than the one we were up to
                _dhcpTransactionId = respId;
                send_DHCP_MESSAGE(DHCP_REQUEST, ((millis() - startTime) / 1000));
                _dhcp_state = STATE_DHCP_REQUEST;
            }
        }
	else if(_dhcp_state == STATE_DHCP_REREQUEST) 
	{
	  send_DHCP_MESSAGE(DHCP_REQUEST, ((millis() - startTime) / 1000), true);
	  prevState = _dhcp_state;
	  _dhcp_state = STATE_DHCP_REQUEST;
	}
	else if(_dhcp_state == STATE_DHCP_REBINDING)
	{
	  send_DHCP_MESSAGE(DHCP_REQUEST, ((millis() - startTime) / 1000));
	  prevState = _dhcp_state; 
	  _dhcp_state = STATE_DHCP_REQUEST;
	}
        else if(_dhcp_state == STATE_DHCP_REQUEST)
        {
            uint32_t respId;
            messageType = parseDHCPResponse(responseTimeout+randomDiff-1000, respId);
            if(messageType == DHCP_ACK)
            {
                _dhcp_state = STATE_DHCP_LEASED;
		_timeOfLease = millis();
                result = 0; // ++
            }
            else if(messageType == DHCP_NAK)
                _dhcp_state = STATE_DHCP_START;
        }
        
        if(messageType == 255)
        {
            messageType = 0;
	    // om den har vart i rerequest så skall den inte börja om i start igen!
            _dhcp_state = prevState;
	    responseTimeout *= 2;
        }

        if(result != 0 && ((millis() - startTime) > _timeout)) 
            break;
    }
    
    // We're done with the socket now
    _dhcpUdpSocket.stop();
    _dhcpTransactionId++;
    
    return result;
}

/*
  returns 0 if it could renew
  returns 1 if it couldn't renew
  return 2 if it didn't have to renew
  
*/
int DhcpClass::renew() {

  //Serial.println(_dhcpT1);
  unsigned long time = millis();

  if (_timeOfLease > time) {
    // millis overflow
    // request new, because we don't know what's left of lease
    if (_renewCount == 0)
      return doT1();
    else if (_renewCount == 1)
      return doT2(); 
    else
      return doLeaseOut(); // T2 has passed, new lease!      
  }
  else if ((time - _timeOfLease)/1000 > _dhcpLeaseTime)
    return doLeaseOut();

  else if ((time - _timeOfLease)/1000 > _dhcpT2 && _renewCount < 2)
    return doT2();

  else if ((time - _timeOfLease)/1000 > _dhcpT1 && _renewCount < 1)
    return doT1();

  else
    return 2;
}

int DhcpClass::doT1()
{
  _dhcp_state = STATE_DHCP_REREQUEST;
  int ret = requestLease();
    // counts renewals, so it doesn't repeat
    _renewCount++;
    if (ret == 1)
      return 2;
    else
      return 0;
}

int DhcpClass::doT2()
{
    _dhcp_state = STATE_DHCP_REBINDING;
    _renewCount++;
    int ret = requestLease();
    if (ret == 1)
      return 2;
    else
      return 0;
}

int DhcpClass::doLeaseOut()
{
    _dhcp_state = STATE_DHCP_START;
    return requestLease();
}

void DhcpClass::presend_DHCP()
{
}

void DhcpClass::send_DHCP_MESSAGE(uint8_t messageType, uint16_t secondsElapsed, bool unicast)
{
    uint8_t buffer[32];
    memset(buffer, 0, 32);
    
    //changed
    IPAddress dest_addr(0,0,0,0);
    //Fix unicast address 
    if (unicast) {
      dest_addr = IPAddress(_dhcpDhcpServerIp);
    }
    else {
      dest_addr = IPAddress( 255, 255, 255, 255 ); // Broadcast address
    }

    if (-1 == _dhcpUdpSocket.beginPacket(dest_addr, DHCP_SERVER_PORT))
    {
        // FIXME Need to return errors
        return;
    }

    buffer[0] = DHCP_BOOTREQUEST;   // op
    buffer[1] = DHCP_HTYPE10MB;     // htype
    buffer[2] = DHCP_HLENETHERNET;  // hlen
    buffer[3] = DHCP_HOPS;          // hops

    // xid
    unsigned long xid = htonl(_dhcpTransactionId);
    memcpy(buffer + 4, &(xid), 4);

    // 8, 9 - seconds elapsed
    buffer[8] = ((secondsElapsed & 0xff00) >> 8);
    buffer[9] = (secondsElapsed & 0x00ff);

    // changed: flags for unicast and broadcast
    unsigned short flags;
    if(unicast)
      flags = htons(DHCP_FLAGSUNICAST);
    else
      flags = htons(DHCP_FLAGSBROADCAST);

    memcpy(buffer + 10, &(flags), 2);

    // ADDED: when its a renew we want to include our ip
    if(unicast)
      memcpy(buffer + 12, &(_dhcpLocalIp),4);
    
    // ciaddr: already zeroed if not renew
    // yiaddr: already zeroed
    // siaddr: already zeroed
    // giaddr: already zeroed

    //put data in W5100 transmit buffer
    _dhcpUdpSocket.write(buffer, 28);

    memset(buffer, 0, 32); // clear local buffer

    memcpy(buffer, _dhcpMacAddr, 6); // chaddr

    //put data in W5100 transmit buffer
    _dhcpUdpSocket.write(buffer, 16);

    memset(buffer, 0, 32); // clear local buffer

    // leave zeroed out for sname && file
    // put in W5100 transmit buffer x 6 (192 bytes)
  
    for(int i = 0; i < 6; i++) {
        _dhcpUdpSocket.write(buffer, 32);
    }
  
    // OPT - Magic Cookie
    buffer[0] = (uint8_t)((MAGIC_COOKIE >> 24)& 0xFF);
    buffer[1] = (uint8_t)((MAGIC_COOKIE >> 16)& 0xFF);
    buffer[2] = (uint8_t)((MAGIC_COOKIE >> 8)& 0xFF);
    buffer[3] = (uint8_t)(MAGIC_COOKIE& 0xFF);

    // OPT - message type
    buffer[4] = dhcpMessageType;
    buffer[5] = 0x01;
    buffer[6] = messageType; //DHCP_REQUEST;

    // OPT - client identifier
    buffer[7] = dhcpClientIdentifier;
    buffer[8] = 0x07;
    buffer[9] = 0x01;
    memcpy(buffer + 10, _dhcpMacAddr, 6);

    // OPT - host name
    buffer[16] = hostName;
    buffer[17] = strlen(HOST_NAME) + 3; // length of hostname + last 3 bytes of mac address
    strcpy((char*)&(buffer[18]), HOST_NAME);

    buffer[24] = _dhcpMacAddr[3];
    buffer[25] = _dhcpMacAddr[4];
    buffer[26] = _dhcpMacAddr[5];

    //put data in W5100 transmit buffer
    _dhcpUdpSocket.write(buffer, 27);

    if(messageType == DHCP_REQUEST)
    {
        buffer[0] = dhcpRequestedIPaddr;
        buffer[1] = 0x04;
        buffer[2] = _dhcpLocalIp[0];
        buffer[3] = _dhcpLocalIp[1];
        buffer[4] = _dhcpLocalIp[2];
        buffer[5] = _dhcpLocalIp[3];

        buffer[6] = dhcpServerIdentifier;
        buffer[7] = 0x04;
        buffer[8] = _dhcpDhcpServerIp[0];
        buffer[9] = _dhcpDhcpServerIp[1];
        buffer[10] = _dhcpDhcpServerIp[2];
        buffer[11] = _dhcpDhcpServerIp[3];

        //put data in W5100 transmit buffer
        _dhcpUdpSocket.write(buffer, 12);
    }
    
    buffer[0] = dhcpParamRequest;
    buffer[1] = 0x06;
    buffer[2] = subnetMask;
    buffer[3] = routersOnSubnet;
    buffer[4] = dns;
    buffer[5] = domainName;
    buffer[6] = dhcpT1value;
    buffer[7] = dhcpT2value;
    buffer[8] = endOption;
    
    //put data in W5100 transmit buffer
    _dhcpUdpSocket.write(buffer, 9);

    _dhcpUdpSocket.endPacket();
}

uint8_t DhcpClass::parseDHCPResponse(unsigned long responseTimeout, uint32_t& transactionId)
{
    uint8_t type = 0;
    uint8_t opt_len = 0;
     
    unsigned long startTime = millis();

    while(_dhcpUdpSocket.parsePacket() <= 0)
    {
        if((millis() - startTime) > responseTimeout)
        {
            return 255;
        }
        delay(50);
    }
    // start reading in the packet
    RIP_MSG_FIXED fixedMsg;
    _dhcpUdpSocket.read((uint8_t*)&fixedMsg, sizeof(RIP_MSG_FIXED));
  
    if(fixedMsg.op == DHCP_BOOTREPLY && _dhcpUdpSocket.remotePort() == DHCP_SERVER_PORT)
    {
        transactionId = ntohl(fixedMsg.xid);
        if(memcmp(fixedMsg.chaddr, _dhcpMacAddr, 6) != 0 || (transactionId < _dhcpInitialTransactionId) || (transactionId > _dhcpTransactionId))
        {
            // Need to read the rest of the packet here regardless
            _dhcpUdpSocket.flush();
            return 0;
        }

        memcpy(_dhcpLocalIp, fixedMsg.yiaddr, 4);

        // Skip to the option part
        // Doing this a byte at a time so we don't have to put a big buffer
        // on the stack (as we don't have lots of memory lying around)
        for (int i =0; i < (240 - (int)sizeof(RIP_MSG_FIXED)); i++)
        {
            _dhcpUdpSocket.read(); // we don't care about the returned byte
        }

        while (_dhcpUdpSocket.available() > 0) 
        {
            switch (_dhcpUdpSocket.read()) 
            {
                case endOption :
                    break;
                    
                case padOption :
                    break;
                
                case dhcpMessageType :
                    opt_len = _dhcpUdpSocket.read();
                    type = _dhcpUdpSocket.read();
                    break;
                
                case subnetMask :
                    opt_len = _dhcpUdpSocket.read();
                    _dhcpUdpSocket.read(_dhcpSubnetMask, 4);
                    break;
                
                case routersOnSubnet :
                    opt_len = _dhcpUdpSocket.read();
                    _dhcpUdpSocket.read(_dhcpGatewayIp, 4);
                    for (int i = 0; i < opt_len-4; i++)
                    {
                        _dhcpUdpSocket.read();
                    }
                    break;
                
                case dns :
                    opt_len = _dhcpUdpSocket.read();
                    _dhcpUdpSocket.read(_dhcpDnsServerIp, 4);
                    for (int i = 0; i < opt_len-4; i++)
                    {
                        _dhcpUdpSocket.read();
                    }
                    break;
                
                case dhcpServerIdentifier :
                    opt_len = _dhcpUdpSocket.read();
                    if( *((uint32_t*)_dhcpDhcpServerIp) == 0 || 
                        IPAddress(_dhcpDhcpServerIp) == _dhcpUdpSocket.remoteIP() )
                    {
                        _dhcpUdpSocket.read(_dhcpDhcpServerIp, sizeof(_dhcpDhcpServerIp));
                    }
                    else
                    {
                        // Skip over the rest of this option
                        while (opt_len--)
                        {
                            _dhcpUdpSocket.read();
                        }
                    }
                    break;

	         case dhcpT1value:
		  opt_len = _dhcpUdpSocket.read();
		  _dhcpUdpSocket.read((uint8_t*)&_dhcpT1, opt_len);
		  _dhcpT1 = ntohl(_dhcpT1);
		  _renewCount = 0;
		    break;
		    
	    case dhcpT2value:
	      opt_len = _dhcpUdpSocket.read();
	      _dhcpUdpSocket.read((uint8_t*)&_dhcpT2, opt_len);
	      _dhcpT2 = ntohl(_dhcpT2);
	      break;
		  
                case dhcpIPaddrLeaseTime :
		  opt_len = _dhcpUdpSocket.read();
		  _dhcpUdpSocket.read((uint8_t*)&_dhcpLeaseTime, opt_len);
		  _dhcpLeaseTime = ntohl(_dhcpLeaseTime);
		  break;

                default :
                    opt_len = _dhcpUdpSocket.read();
                    // Skip over the rest of this option
                    while (opt_len--)
                    {
                        _dhcpUdpSocket.read();
                    }
                    break;
            }
        }
    }

    // Need to skip to end of the packet regardless here
    _dhcpUdpSocket.flush();

    return type;
}

IPAddress DhcpClass::getLocalIp()
{
  return IPAddress(_dhcpLocalIp);
}

IPAddress DhcpClass::getSubnetMask()
{
    return IPAddress(_dhcpSubnetMask);
}

IPAddress DhcpClass::getGatewayIp()
{
    return IPAddress(_dhcpGatewayIp);
}

IPAddress DhcpClass::getDhcpServerIp()
{
    return IPAddress(_dhcpDhcpServerIp);
}

IPAddress DhcpClass::getDnsServerIp()
{
    return IPAddress(_dhcpDnsServerIp);
}

