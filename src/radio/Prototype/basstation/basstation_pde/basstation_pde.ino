#include <Radio.h>

Radio radio;
const int sendDelay = 2500;

void setup() {
  radio = Radio(&Serial); 
}

void loop() {
  radio.send("Kermit the Frog\004");
  delay(sendDelay);
}

