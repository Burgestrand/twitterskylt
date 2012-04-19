#include <Radio.h>
#include <Coordinator.h>
#include "Arduino.h"

#include <EthernetClient.h>
#include <IPAddress.h>

#include <HTTP.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include <Formatting.h>
#include <Time.h>
#include <SPI.h>
#include <Ethernet.h>
#include <AccConfig.h>
#include <SD.h>
#include <Common.h>
#include <TweetParser.h>

Coordinator coordinator;

const int errorPin = 22;
const int assocPin = 24;
unsigned long int blinkTimer = 0;
unsigned long int tempTimer = 0;
boolean assocPinStatus = false;
SoftwareSerial nss = SoftwareSerial(30, 32);
uint8_t sendData[120];
AccConfig config;


void setup(void)
{
	Serial.begin(9600);
	pinMode(errorPin, OUTPUT);
	pinMode(assocPin, OUTPUT);
for(int i=0; i<72; i++) sendData[i] = '1';
for(int i=72; i<120; i++) sendData[i] = '2';
	coordinator.begin(Serial1);

	int configStatus = config.begin("KONF1.TXT");
	if (configStatus > 0) {
		signalError(true);
		abort();
	}
	Serial.println(config.getUsername());
	Serial.println(config.getUsername());
	Serial.println(config.getQuery());
	Serial.println(config.getQuery());
	byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF9, 0x83 };
	int ethernetStatus = Ethernet.begin(mac);
	if (ethernetStatus > 0) {
		signalError(true);
		abort();
	}
	  attachInterrupt(0, pairUp, FALLING);   
	  nss.begin(9600);
	  nss.println(" ");
	  nss.println(" ");
	  nss.println("***** RESET *****");
	  nss.println(" ");
		signalError(false);
		signalAssoc(false);	  
	  blinkTimer = 0;
	//coordinator.setData(sendData, sizeof(sendData));
}

void loop(void)
{
	//uint8_t ip[4] = {192, 168, 0, 1};
  //HTTP http(ip, 10);
  Ethernet.renew();
  delay(10);
  uint8_t st = coordinator.tick();
  uint8_t s = coordinator.getState();
  assocLed(s);
  printState(s);
  if(st == 1 || st == 3) {
	signalError(true);    
	//digitalWrite(errorPin, HIGH);
  }
  else {
	signalError(false);
     //digitalWrite(errorPin, LOW); 
  }

	boolean parse_complete = false;

	char json[] = {"{\"query\": \"from%3Aajstream+%23egypt\","
					"\"results\": [{\"created_at\": \"Mon, 05 Mar 2012 01:00:16 +0000\","
									"\"metadata\": {\"result_type\": \"recent\"},"
									"\"text\": \"Protests at the German University of #Cairo #GUC #Egypt - http://t.co/PeZryCQR\"}],"
					"\"results_per_page\": 1}"};

	char * shared_buffer = (char *) malloc(256 * sizeof(char));
	char * text = (char *) malloc(160 * sizeof(char));
	char * date = (char *) malloc(32 * sizeof(char));
	TweetParser parser(shared_buffer, text, 160, date, 32);
	int json_start = 0;
	while (!parse_complete) {
		put_in_buf(json_start, 30, json, shared_buffer);
		parse_complete = parser.parse(256); 
	}

}

void put_in_buf(int begin, int length, char from[], char * to) {
	for (int i = begin; i < length; i++)
		to[i] = from[i];
} 

void assocLed(uint8_t state) {
  if(state == 7) {
    blinkPairUp();
  }
  else {
    if(coordinator.getAssoc()) {
		signalAssoc(true);      
		//digitalWrite(assocPin, HIGH);
    }
    else {
		signalAssoc(false);
       //digitalWrite(assocPin, LOW);
    }
  }  
}

void pairUp() {
  coordinator.pairUp();
}

void blinkPairUp() {
  if(millis() >= blinkTimer) {
	digitalWrite(assocPin, (assocPinStatus = !assocPinStatus));
        blinkTimer = millis() + 500;
  }
}

void printState(uint8_t s) {
  nss.print("State [");
  nss.print(s);
  nss.print("]  =  [");
  nss.print(coordinator.getStateName(s));
  nss.println("]");
}

void signalError(boolean on) {
	if (on)
		digitalWrite(errorPin, HIGH);
	else
		digitalWrite(errorPin, LOW);
}

void signalAssoc(boolean on) {
	if (on)
		digitalWrite(assocPin, HIGH);
	else
		digitalWrite(assocPin, LOW);
}

void abort() {
	for (;;);
}

