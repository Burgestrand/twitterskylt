
#include <avr/interrupt.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include <EndDevice.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <Display.h>
#include <Sleep.h>

// TODO: 
// Radio error handling
// Use leds properly

// The maximum size of a message from the base station
// including date, nullbyte, hyphens, twitter message
// and newlines.
#define MAX_MSG_SIZE 168

// Pins
// Display related
#define DISPLAY_SELECT_PIN 10
#define DISPLAY_BUSY_PIN 8
#define DISPLAY_RESET_PIN 9

// Software serial pins for debugging
#define SERIAL_RX 7
#define SERIAL_TX 6

// Leds
// The red led
#define LED_1 A0
// The yellow led
#define LED_2 A1

// Buttons
// The button closest to the leds
#define BUTTON_1 2
// The button furthest from the leds
#define BUTTON_2 3

// XBee sleep request/status
#define SLEEP_RQ_PIN 4
#define SLEEP_STATUS_PIN 5

// Global variables
// The display
Display disp(DISPLAY_SELECT_PIN, DISPLAY_BUSY_PIN, DISPLAY_RESET_PIN);

// The radio
EndDevice radio(SLEEP_RQ_PIN, SLEEP_STATUS_PIN);

// The serial connection used for debug output
SoftwareSerial ss(SERIAL_RX, SERIAL_TX);

// True if the join button has been pressed, but not yet handled
volatile bool joinPressed = false;

// True if the update button has been pressed, but not yet handled
volatile bool updatePressed = false;

// Store old message to be able to notice when a message fetched from the base station is actually new.
char message[MAX_MSG_SIZE] = {0};
// True if a new message has been received, and should be printed.
bool new_msg = false;

// Helpers
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

void invalidate_data(void) {
  message[0] = 0;
  new_msg = false;
}

// Usual arduino functions
void setup () {
  // Activity  led indicating that the arduino is awake
  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, HIGH);
  
  // Initialize the display
  disp.begin();  
  
  // Initialize debug serial port
  ss.begin(9600);
  
  // Initialize the radio
  radio.begin(9600);
  
  // Initialize the sleep timer
  sleep_begin();
  
  // Setup the buttons
  attachInterrupt(0, joinButtonPressed, RISING);
  attachInterrupt(1, updateButtonPressed, RISING);

  // Show that we're done initializing
  disp.write("Initialized");
}

void loop () {
  // Handle button presses
  // Pair button
  if (joinPressed) {
    joinPressed = false;
    debug("JOINBTN NOTICED");
    disp.write("Trying to join\nnetwork");
    invalidate_data();
    radio.joinNetwork();
  }

  // Necessary for the radio library to work
  switch (radio.tick()) {
    case TICK_ASSOC_FAIL:
      // Association failed (note: not pairing)
      invalidate_data();
      disp.write("Assoc fail");
      break;
    case TICK_JOIN_NOT_DELIVERED:
      // Could not deliver join message
      invalidate_data();
      disp.write("Join:\n No delivery");
      break;
    case TICK_JOIN_TIMEOUT:
      invalidate_data();
      disp.write("Join:\n Timeout");
      break;
    case TICK_JOIN_OK:
      invalidate_data();
      disp.write("Joined!");
      break;
    case TICK_UPDATE_NO_DELIVERY:
      invalidate_data();
      disp.write("Update:\n No delivery");
      break;
    case TICK_UPDATE_TIMEOUT:
      invalidate_data();
      disp.write("Update:\n Timeout");
      break;
    case TICK_NEW_MSG:
      // Update message buffer if a different message is recieved and set a flag.
      // We can't update the display here as it will block for some non-insignificant time.
      debug("GOT A MSG");
      if (strncmp(message, (char *) radio.getData(), MAX_MSG_SIZE-1)) {
          debug("GOT A NEW MSG");
          strncpy(message, (char *) radio.getData(), MAX_MSG_SIZE-1);
          new_msg = true;
      }
      break;
    case TICK_SLEEPING:
      // Send the new message to the display if there is one.
      // The blocking write operation is safe here as the radio is sleeping.
      if (new_msg) {
          debug("PRINTING MESSAGE");
          disp.write(message);
          new_msg = false;
      }
      // Radio is sleeping and we have nothing to do; lets sleep!
      digitalWrite(LED_1, LOW);
      {
        uint8_t sleepRounds = 3;
        // Sleep until we've sleept long enough or a button has been pressed.
        while (sleepRounds-- && !joinPressed && !updatePressed) {
          sleep();
        }
      }
      // If the join button was pressed we shouldn't update the message.
      if (!joinPressed) {
        digitalWrite(LED_1, HIGH);
        radio.wakeup();
        radio.getNewestMessage();
      }
      updatePressed = false;
      break;
    case TICK_UNKNOWN_ERROR:
      invalidate_data();
    case TICK_JOIN_BAD_MSG:
      invalidate_data();
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

