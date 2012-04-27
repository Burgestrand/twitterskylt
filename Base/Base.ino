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
#include <MemoryFree.h>

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
	pinMode(53, OUTPUT);
	coordinator.begin(Serial1);
	pinMode(10, OUTPUT);
	pinMode(4, OUTPUT);
	digitalWrite(10, HIGH);
	int configStatus = config.begin("KONF1.TXT");
	if (configStatus > 0) {
		Serial.println("SD fail");
		showError();
		abort();
	}
	digitalWrite(4, HIGH);
	byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF9, 0x83 };
	int ethernetStatus = Ethernet.begin(mac);
	if (ethernetStatus > 0) {
		Serial.println("eth fail");
		showError();
		abort();
	}
	
	Serial.println("everything ok!");
	attachInterrupt(0, pairUp, FALLING);
	hideError();
	hideAssoc();
	blinkTimer = 0;

	//setupTimezone();
}

void setupTimezone()
{
  size_t buffer_size = 1024;
  HTTP *client = new HTTP(buffer_size);

  Serial.println("Get: ");
  int get = client->get(IPAddress(199, 59, 150, 9), "/1/users/show.json", 2, "screen_name", "door_sign");

  if (get == 0)
  {
    Serial.println("Get success!");
    Serial.println();
    Serial.println();

    const char *read_data = NULL;
    int32_t read_length = 0;

    while (true)
    {
      read_data = client->tick(&read_length);

      Serial.print("tick: ");
      Serial.println(read_length, DEC);
      Serial.println(read_data);
      Serial.println();

      delay(1000);
    }
  }
  else
  {
    Serial.println("Get error");
    Serial.println(get);
    Serial.println();
  }

	/*
	URL: "api.twitter.com/1/users/show.json"
	Param 1: "screen_name"
	Value 1: config.getUsername()
	Eventuellt returnera statuskod om det misslyckas och då antingen visa fel och avsluta eller välja default.
	*/
}

void loop(void)
{
	ethernetTick();
	radioTick();
	Serial.println("loop");
	tweetTick();
	delay(1000);
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

HTTP *httpClient = NULL;
unsigned long nextRequest = 0;
unsigned long updateInterval = 60 * 1000; // 1 minute interval

#define HTTP_BUFFER_SIZE 200

void tweetTick()
{
	char * text;
	char * date;
  if (millis() >= nextRequest)
  {
	text = (char *) calloc(161, sizeof(char));
	date = (char *) calloc(41, sizeof(char));
	TweetParser parser = TweetParser(text, 160, date, 40);
	Serial.println("lets get");
    // calculate next time to do the request
    nextRequest += updateInterval;
    // httpClient.~HTTP();
    httpClient = new HTTP(HTTP_BUFFER_SIZE);
    // new request
	int8_t ret = httpClient->get(IPAddress(199,59,148,201), "/search.json", 6, "q", "from:door_sign", "result_type", "recent", "rpp", "1");
	int32_t length = 0;
	bool finished = false;
	while (!finished) {
		const char * buffer = httpClient->tick(&length);
		if (length > 0) {
			fixJSON(buffer);
			finished = parser.parse(buffer, length);
		}
	}
	httpClient->rem();
	parser.del();
	char * result = Formatting::format(text,date,3600);
	coordinator.setData((uint8_t *)result, (uint8_t)strlen(result));
	Serial.println(result);
  }
  else
  {
		Serial.println("no");
  }
}

// removes \ to pass yajl
void fixJSON(const char * str) {
	char * ptr = (char *) str;
	while(*ptr) {
		if (*ptr == '\\') {
			*ptr = '/';
		}
		ptr++;
	}
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
