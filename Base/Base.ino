#include <Radio.h>
#include <Coordinator.h>
#include <Ethernet.h>
#include <HTTP.h>
#include "Arduino.h"
#include <SoftwareSerial.h>
#include <XBee.h>
#include <Formatting.h>
#include <Time.h>
#include <SPI.h>

Coordinator coordinator;

const int errorPin = 22;
const int assocPin = 24;
unsigned long int blinkTimer = 0;
unsigned long int tempTimer = 0;
boolean assocPinStatus = false;
SoftwareSerial nss = SoftwareSerial(30, 32);
uint8_t sendData[120];


void setup(void)
{
for(int i=0; i<72; i++) sendData[i] = '1';
for(int i=72; i<120; i++) sendData[i] = '2';
	coordinator.begin(Serial1);
	byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xF9, 0x83 };
	//Ethernet.begin(mac);
	  attachInterrupt(0, pairUp, FALLING);   
	  nss.begin(9600);
	  nss.println(" ");
	  nss.println(" ");
	  nss.println("***** RESET *****");
	  nss.println(" ");
	  pinMode(errorPin, OUTPUT);
	  pinMode(assocPin, OUTPUT);
	  digitalWrite(errorPin, LOW);
	  digitalWrite(assocPin, LOW);
	  blinkTimer = 0;
	//coordinator.setData(sendData, sizeof(sendData));
}

void loop(void)
{
    //Ethernet.renew();
  delay(10);
  uint8_t st = coordinator.tick();
  uint8_t s = coordinator.getState();
  assocLed(s);
  printState(s);
  if(st == 1 || st == 3) {
    digitalWrite(errorPin, HIGH);
  }
  else {
     digitalWrite(errorPin, LOW); 
  }
   HTTP::tick(&coordinator);
}

void assocLed(uint8_t state) {
  if(state == 7) {
    blinkPairUp();
  }
  else {
    if(coordinator.getAssoc()) {
      digitalWrite(assocPin, HIGH);
    }
    else {
       digitalWrite(assocPin, LOW);
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

