#include <Radio.h>
#include <Coordinator.h>
#include "Arduino.h"

#include <EthernetClient.h>
#include <IPAddress.h>

#include <HTTP.h>
#include <Radio.h>
#include <Utilities.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include <Formatting.h>
#include <Time.h>
#include <SPI.h>
#include <Ethernet.h>
#include <AccConfig.h>
#include <SD.h>
#include <TweetParser.h>

Coordinator coordinator;

const int errorPin = 22;
const int assocPin = 24;
unsigned long int blinkTimer = 0;
unsigned long int tempTimer = 0;
boolean assocPinStatus = false;
AccConfig config;

void setup(void)
{
	Serial.begin(9600);
	pinMode(errorPin, OUTPUT);
	pinMode(assocPin, OUTPUT);
	coordinator.begin(Serial1);
/*
	int configStatus = config.begin("KONF1.TXT");
	if (configStatus > 0) {
		showError();
		abort();
	}
	
	byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF9, 0x83 };
	int ethernetStatus = Ethernet.begin(mac);
	if (ethernetStatus > 0) {
		showError();
		abort();
	}
	*/
	attachInterrupt(0, pairUp, FALLING);
	hideError();
	hideAssoc();	  
	blinkTimer = 0;
	
	setupTimezone();
}

void setupTimezone()
{
	/*
	URL: "api.twitter.com/1/users/show.json"
	Param 1: "screen_name"
	Value 1: config.getUsername()
	Eventuellt returnera statuskod om det misslyckas och då antingen visa fel och avsluta eller välja default.
	*/
}

void loop(void)
{
	//ethernetTick();
	//radioTick();
	Serial.println("loop");
	tweetTick();
}

void ethernetTick()
{
	Ethernet.renew();
}

void radioTick()
{
	uint8_t st = coordinator.tick();
	uint8_t s = coordinator.getState();
	assocLed(s);
	if (st == 1 || st == 3) {
		showError();
	}
	else {
		hideError(); 
	}
}

//TweetParser parser = NULL;
HTTP *httpClient = NULL;
unsigned long nextRequest = 0;
unsigned long updateInterval = 60 * 1000; // 1 minute interval

#define HTTP_BUFFER_SIZE 1024

void tweetTick()
{
  if (millis() >= nextRequest)
  {
    // calculate next time to do the request
    nextRequest += updateInterval;
    // httpClient.~HTTP();
    httpClient = new HTTP(HTTP_BUFFER_SIZE);
    // new request
  }
  else
  {

  }

  /*
	#define UPDATE_INTERVAL 10000
	char *buffer = (char *) calloc(SIZE, sizeof(char));
	char *text = (char *) calloc(161, sizeof(char));
	char *date = (char *) calloc(41, sizeof(char));

	TweetParser parser = TweetParser(buffer, text, 161, date, 40);

  lastRequest = millis();

  char * result = Formatting::format(text, date, 0);
  Serial.println("formatted tweet:");
  Serial.println(result);
  */
}

void assocLed(uint8_t state) {
  if(state == 7) {
    blinkPairUp();
  }
  else {
    if(coordinator.getAssoc()) {
		showAssoc();
    }
    else {
		hideAssoc();
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

void showError()
{
	digitalWrite(errorPin, HIGH);
}

void hideError()
{
	digitalWrite(errorPin, LOW);
}

void showAssoc()
{
	digitalWrite(assocPin, HIGH);
}

void hideAssoc()
{
	digitalWrite(assocPin, LOW);
}

// Change this if we implement the watchdog reset timer.
void abort() {
	for (;;);
}
