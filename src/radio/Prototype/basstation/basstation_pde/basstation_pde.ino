#include <Sleep.h>
#include <Radio.h>

Radio radio;
Sleep sleep;
const int sendDelay = 2500;

void setup() {
  pinMode(2, OUTPUT);
  radio = Radio(&Serial);
  sleep.begin();
  digitalWrite(2, LOW);
}

void loop() {
      //digitalWrite(2, HIGH);
      //delay(1000);
      //digitalWrite(2, LOW);
}

ISR(TIMER1_COMPA_VECT) {
      digitalWrite(2, HIGH);
      //delay(1000);
      //digitalWrite(2, LOW);
  /*    
   if(!sleep.shouldWakeUp()) {
      sleep.sleep();
      digitalWrite(2, HIGH);
      delay(100);
      digitalWrite(2, LOW);
    } 
    else {
      digitalWrite(2, HIGH);
      delay(1000);
      digitalWrite(2, LOW);    
    }
    */
}
