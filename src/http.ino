#include <stdio.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Dns.h>
#include <HTTPClient.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF9, 0x83 };
char twitterName[] = "search.twitter.com";

void setup() {
	pinMode(53, OUTPUT);
	Serial.begin(9600);
	
	if (Ethernet.begin(mac)) {
		DNSClient dns;
		IPAddress twitterIP;
		dns.begin(Ethernet.dnsServerIP());
		boolean ipFound = dns.getHostByName(twitterName, twitterIP);
		
		if (ipFound) {
			Serial.print("Twitter has IP ");
			Serial.println(twitterIP);
			
			// Apparently, an IPAddress can be cast to an integer.
			HTTPClient client(twitterName, (uint8_t*) &twitterIP);
			client.debug(1);
			FILE* response = client.getURI("/search.json?q=from:ajstream&result_type=recent&rpp=1");
			
			/*String utf = "";
			char c = (char) fgetc(response);
			utf += c;
			Serial.print(utf);
			Serial.println("/END");*/
			client.closeStream(response);
		} else {
			Serial.println("Server could not be found.");
		}
	} else {
		Serial.println("DHCP did not work.");
	}
}

void loop() {
	
}

void requestTweet() {
	/*if (client.connect(serverName, WEB_PORT)) {
		client.println("GET /search.json?q=from:ajstream&result_type=recent&rpp=1 HTTP/1.1");
		client.print("HOST: "); client.println(serverName);
		client.println();
	} else {
		Serial.println("Client could not connect.");
	}*/
}
