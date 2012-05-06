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

//
// Function prototypes
//
void fail_hard(const char *error);

//
// Defines
//
#define HTTP_BUFFER_SIZE 128

#define DESTROY(class_pointer) do { \
    if (class_pointer != NULL)      \
    {                               \
      class_pointer->teardown();    \
      delete class_pointer;         \
    }                               \
  } while(0)
#define DEBUG(x) Serial.println((x))

//
// Global variables
//

Coordinator coordinator;

int g_utc_offset = 0;

const int errorPin = 22;
const int assocPin = 24;
unsigned long int blinkTimer = 0;
unsigned long int tempTimer = 0;
boolean assocPinStatus = false;
AccConfig config;

//
// Main program
//

void setup(void)
{
  // Setup Serial interface to 9600 baud
  Serial.begin(9600);

  // Activate pins for output and show life signs on startup
  pinMode(errorPin, OUTPUT);
  pinMode(assocPin, OUTPUT);
  showError();
  showAssoc();

  // TODO: what pins are these magic numbers for?
  // 53, 10, 4

  pinMode(53, OUTPUT);
  coordinator.begin(Serial1);
  pinMode(10, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(10, HIGH);

  // Read user configuration
  int configStatus = config.begin("KONF0.TXT");
  if (configStatus > 0)
  {
    fail_hard("SD Fail");
  }

  digitalWrite(4, HIGH);
  byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF9, 0x83 };
  int ethernetStatus = Ethernet.begin(mac);
  if (ethernetStatus > 0)
  {
    fail_hard("ETH fail");
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
  HTTP *client = new HTTP("api.twitter.com", HTTP_BUFFER_SIZE);
  http_error_t error = client->get(IPAddress(199, 59, 150, 9), "/1/users/show.json", 2, "screen_name", "door_sign");

  if (error != HTTP_OK)
  {
    DEBUG(client->explainError(error));
  }
  else
  {
    UserParser parser = UserParser(&g_utc_offset);
    const char *read_data = NULL;
    int32_t read_length = 0;

    bool finished = false;
    while (!finished)
    {
      read_data = client->tick(&read_length);
      if (read_length > 0)
      {
        finished = parser.parse(read_data, read_length);
      }
    }

    parser.teardown();
  }

  DESTROY(client);

  DEBUG("UTC Offset: ");
  DEBUG(g_utc_offset);
}

void loop(void)
{
  ethernetTick();
  radioTick();
  DEBUG("loop");
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

unsigned long nextRequest = 0;
unsigned long updateInterval = 60 * 1000; // 1 minute interval

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

    // calculate next time to do the request
    nextRequest += updateInterval;

    DEBUG("Fetching new tweet");
    HTTP *httpClient = new HTTP("search.twitter.com", HTTP_BUFFER_SIZE);
    http_error_t error = httpClient->get(IPAddress(199,59,148,201), "/search.json", 6, "q", "from:door_sign", "result_type", "recent", "rpp", "1");

    if (error != 0)
    {
      // Show error somehow
      DEBUG(httpClient->explainError(error));
      goto cleanup;
    }

    while (!finished)
    {
      buffer = httpClient->tick(&length);
      if (length > 0)
      {
        finished = parser.parse(buffer, strlen(buffer));
      }
    }

    DEBUG(date);
    DEBUG(text);

    result = Formatting::format(text, date, g_utc_offset);
    coordinator.setData((uint8_t *)result, (uint8_t)strlen(result));
    DEBUG(result);

cleanup:
    DESTROY(httpClient);

    parser.teardown();
  }
  else
  {
    DEBUG("tweetTick(): nothing to do");
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

/*
 * Allows us to fail with an error message. This will hang
 * the Arduino and allow the watchdog to reset it.
 */
void fail_hard(const char *error)
{
  Serial.println(error);
  showError();
  hideAssoc();
  abort();
}
