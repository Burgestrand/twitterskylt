
// #include "Common/Common.h" /* Not needed? */
#include <avr/interrupt.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <EndDevice.h>
#include <Display.h>
#include <Sleep.h>

// TODO: 
// Debouncing update/join buttons
// Handle xbee SLEEP pins better.
// xbee error handling!

// PINS /////////////////////////////////////////////////////////////////////////////////
// Display related
#define DISP_RS 10
#define DISP_EN 11
#define DISP_DATA 4

// Software serial for debugging
#define SERIAL_RX 9
#define SERIAL_TX 8

// Leds
// The red led
#define LED_1 A0
// The yellow led
#define LED_2 A1

// Buttons
// The one closest to the leds
#define BUTTON_1 2
// The one furthest from the leds
#define BUTTON_2 3

// XBee sleep request/status
#define SLEEP_RQ_PIN 12
#define SLEEP_STATUS_PIN 13

// GLOBAL VARIABLES ////////////////////////////////////////////////////////////////
// The display
Display disp(DISP_RS, DISP_EN, DISP_DATA, DISP_DATA+1, DISP_DATA+2, DISP_DATA+3);

// The radio
EndDevice radio(SLEEP_RQ_PIN, SLEEP_STATUS_PIN);

// The serial connection used for debug output
SoftwareSerial ss(SERIAL_RX, SERIAL_TX);

// State for the button debounce
long latestJoinDown;
long latestUpdateDown;

// True if the join button has been pressed, but not yet handled
volatile bool joinPressed = false;

// True if the update button has been pressed, but not yet handled
volatile bool updatePressed = false;

// HELPER FUNCTIONS /////////////////////////////////////////////////////////////////
// Debug output callback
void debug(char *msg) {
  ss.write("DEBUG: ");
  ss.write(msg);
  ss.write("\n");
}

// ISR for handling join button presses
void joinButtonPressed(void) {
  joinPressed = true;
}

// ISR for handling update button presses
void updateButtonPressed(void) {
  updatePressed = true;
}

// REGULAR ARDUINO STUFF /////////////////////////////////////////////////////////////
void setup () {
  // Activity  led indicating that the loop runs
  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, HIGH);
  
  // Initialize the display
  disp.begin(16, 2);  
  
  // Initialize debug serial port
  ss.begin(9600);
  
  // Setup update button and SW debounce
  latestJoinDown = millis();
  latestUpdateDown = millis();
  
  // Initialize the radio
  radio.begin(9600);
  
  // Initialize sleep timer
  sleep_begin();
  
  // Setup the buttons
  attachInterrupt(0, joinButtonPressed, RISING);
  attachInterrupt(1, updateButtonPressed, RISING);

  // Show that we're done initializing
  disp.write("Initialized\004");
}

void loop () {
  // Handle button presses, with debounce.
  /*uint8_t down;*/
  // Pair button
  if (joinPressed) {
    joinPressed = false;
    debug("JOINBTN NOTICED");
    disp.write("Trying to join\nnetwork\004");
    radio.joinNetwork();
  }
  if (updatePressed) {
    updatePressed = false;
    debug("UPDATEBTN NOTICED");
    disp.write("Updating...\004");
    radio.getNewestMessage();
  }
  /*noInterrupts();
  down = joinPressed;
  joinPressed = false;
  interrupts();
  if (down && latestJoinDown < millis() - 500) {
    debug("JOINBTN NOTICED");
    disp.write("Trying to join\nnetwork\004");
    radio.joinNetwork();
  }
  if (down) {
    latestJoinDown = millis();
  }*/
  
  // Update button
  /*noInterrupts();
  down = updatePressed;
  updatePressed = false;
  interrupts();
  if (down && latestUpdateDown < millis() - 500) {
    debug("UPDATEBTN NOTICED");
    disp.write("Updating...\004");
    radio.getNewestMessage();
  }
  if (down) {
    latestUpdateDown = millis();
  }*/
  
  // Necessary for the radio library to work
  switch (radio.tick()) {
    case TICK_ASSOC_FAIL:
      // Association failed (note: not pairing)
      disp.write("Assoc fail\004");
      break;
    case TICK_JOIN_NOT_DELIVERED:
      // Could not deliver join message
      disp.write("Join:\n No deliver\004");
      break;
    case TICK_JOIN_TIMEOUT:
      disp.write("Join:\n Timeout\004");
      break;
    case TICK_JOIN_OK:
      disp.write("Joined!\004");
      break;
    case TICK_UPDATE_NO_DELIVERY:
      disp.write("Update:\nNo deliver\004");
      break;
    case TICK_UPDATE_TIMEOUT:
      disp.write("Update:\nTimeout\004");
      break;
    case TICK_NEW_MSG:
      disp.write((char *) radio.getData());
      disp.write('\004');
      debug("GOT NEW MSG");
      break;
    case TICK_SLEEPING:
      //disp.write("Sleeping\004");
      // Radio is sleeping, and we have nothing to do; lets sleep!
      digitalWrite(LED_1, LOW);
      {
        uint8_t sleepRounds = 3;
        while (sleepRounds-- && !joinPressed) {
          sleep();
        }
      }
      if (!joinPressed) {
        digitalWrite(LED_1, HIGH);
        radio.wakeup();
        radio.getNewestMessage();
      }
      break;
    case TICK_UNKNOWN_ERROR:
    case TICK_JOIN_BAD_MSG:
    case TICK_OK:
    default:
      // Do nothing
      break;
  }
  
  // Keep the debug output speed down
  delay(10);
}

// Watchdog is only for waking the microcontroller, so the body is empty.
EMPTY_INTERRUPT(WDT_vect)

