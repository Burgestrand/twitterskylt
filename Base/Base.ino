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
#include <UserParser.h>

Coordinator coordinator;

const int errorPin = 22;
const int assocPin = 24;
unsigned long int blinkTimer = 0;
unsigned long int tempTimer = 0;
int timezone;
boolean assocPinStatus = false;
AccConfig config;

void setup(void)
{
	Serial.begin(9600);
	pinMode(errorPin, OUTPUT);
	pinMode(assocPin, OUTPUT);

        //Life sign on startup
        showError();
        showAssoc();

	pinMode(53, OUTPUT);
	coordinator.begin(Serial1);
	pinMode(10, OUTPUT);
	pinMode(4, OUTPUT);
	digitalWrite(10, HIGH);
	int configStatus = config.begin("KONF0.TXT");
	if (configStatus > 0) {
		Serial.println("SD fail");
		justShowError();
		abort();
	}


	digitalWrite(4, HIGH);
	byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF9, 0x83 };
	int ethernetStatus = Ethernet.begin(mac);
	if (ethernetStatus > 0) {
		Serial.println("eth fail");
		justShowError();
		abort();
	}

	Serial.println("everything ok!");
	attachInterrupt(0, pairUp, FALLING);
	hideError();
	hideAssoc();
	blinkTimer = 0;

	setupTimezone();
}

void setupTimezone()
{
  size_t buffer_size = 1024;
  HTTP *client = new HTTP("api.twitter.com", buffer_size);

  Serial.println("Get: ");
  int get = client->get(IPAddress(199, 59, 150, 9), "/1/users/show.json", 2, "screen_name", "door_sign");

  if (get == 0)
  {
    Serial.println("Get success!");
    Serial.println();
    Serial.println();

    const char *read_data = NULL;
    int32_t read_length = 0;
	
	UserParser parser = UserParser(&timezone);
	bool finished = false;
    while (!finished)
    {
      read_data = client->tick(&read_length);
	  if (read_length > 0)
	  	finished = parser.parse(read_data, read_length);
      Serial.print("tick: ");
      //Serial.println(read_length, DEC);
      //Serial.println(read_data);
      //Serial.println();

      //delay(1000);
    }
	Serial.println("timezone:");
	Serial.println(timezone);
  }
  else
  {
    Serial.println("Get error");
    Serial.println(get);
    Serial.println();
	//couldn't connect to twitter
	timezone = 0;
  }
  client->destroy();

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
	delay(10); // Rate limit for output
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
  char *text, *date, *result;
  bool finished = false;
  int32_t length = 0;
  const char *buffer;

  if (millis() >= nextRequest)
  {
    text = ALLOC_STR(160);
    date = ALLOC_STR(40);
    TweetParser parser = TweetParser(text, 160, date, 40);
    Serial.println("lets get");
    // calculate next time to do the request
    nextRequest += updateInterval;
    httpClient = new HTTP("search.twitter.com", HTTP_BUFFER_SIZE);
    // new request
    http_error_t error = httpClient->get(IPAddress(199,59,148,201), "/search.json", 6, "q", "from:door_sign", "result_type", "recent", "rpp", "1");

    if (error != 0)
    {
      // Show error somehow
      Serial.println(httpClient->explainError(error));
      goto cleanup;
    }

    while (!finished) {
      buffer = httpClient->tick(&length);
      if (length > 0) {
        finished = parser.parse(buffer, strlen(buffer));
      }
    }
    Serial.println(date);
    Serial.println(text);

    result = Formatting::format(text, date, timezone);
    coordinator.setData((uint8_t *)result, (uint8_t)strlen(result));
    Serial.println(result);

    cleanup:
      httpClient->destroy();
      parser.del();
  }
  else
  {
    Serial.println("no");
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

void justShowError()
{
        showError();
        hideAssoc();
}

// Change this if we implement the watchdog reset timer.
void abort() {
	for (;;);
}
