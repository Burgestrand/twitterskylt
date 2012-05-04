
#include <avr/interrupt.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include <EndDevice.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <Display.h>
#include <Sleep.h>

// TODO: 
// Debouncing update/join buttons?
// Radio error handling

// Constants related to battery level reading
#define V_RES 0.004882814
#define V_TH 0.7
#define V_LOW 3.3
#define Q_DROP 408

// Pins
// Display related
#define DISPLAY_SELECT_PIN 10
#define DISPLAY_BUSY_PIN 8
#define DISPLAY_RESET_PIN 9

// Software serial pins for debugging
#define SERIAL_RX 7
#define SERIAL_TX 6
// Join Button
#define JOIN_BUTTON 2
// Update Button
#define UPDATE_BUTTON 3
// Red LED
#define ERROR_LED A1
// Yellow LED
#define BATTERY_LED A2
// Green LED
#define ASSOC_LED A3

// Analog battery reading
#define BATT_A A0
//Voltage divider control
#define VDIV_D 6


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

// True if the battery level is low
bool lowBattery = false;
// Analog value read from voltage divider
float batteryLevel = 0;

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

float getPinVoltage(uint8_t pin) {
	return (analogRead(pin) * V_RES);
}

float getBatteryVoltage(uint8_t pin) {
  // Voltage divider divides voltage by 2
  // Value from analog port has resolution of 49mV (5V/1024)
  // Also adjusts for voltage drop across MOSFET (approximation)
  int batteryLevel = analogRead(0);
  float Vbatt = (2*batteryLevel*V_RES) - 0.04*(batteryLevel/Q_DROP);
  return Vbatt;
}

// Usual arduino functions
void setup () {
  pinMode(ASSOC_LED, OUTPUT);
  digitalWrite(ASSOC_LED, LOW);
  pinMode(ERROR_LED, OUTPUT);
  digitalWrite(ERROR_LED, LOW);
  pinMode(BATTERY_LED, OUTPUT);
  digitalWrite(ERROR_LED, LOW);

  pinMode(VDIV_D, OUTPUT);
  digitalWrite(VDIV_D, LOW);
  pinMode(BATT_A, INPUT);
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
    radio.joinNetwork();
  }
  
  digitalWrite(VDIV_D, HIGH);
  batteryLevel = getBatteryVoltage(BATT_A);
  delay(1);
  digitalWrite(BATTERY_LED, (batteryLevel <= V_LOW ? HIGH : LOW));
  
  // Necessary for the radio library to work
  switch (radio.tick()) {
    case TICK_ASSOC_FAIL:
      // Association failed (note: not pairing)
      disp.write("Assoc fail");
      digitalWrite(ERROR_LED, HIGH);
      break;
    case TICK_JOIN_NOT_DELIVERED:
      // Could not deliver join message
      disp.write("Join:\n No delivery");
      digitalWrite(ERROR_LED, HIGH);
      break;
    case TICK_JOIN_TIMEOUT:
      disp.write("Join:\n Timeout");
      digitalWrite(ERROR_LED, HIGH);
      break;
    case TICK_JOIN_OK:
      disp.write("Joined!");
      digitalWrite(ASSOC_LED, HIGH);
      break;
    case TICK_UPDATE_NO_DELIVERY:
      disp.write("Update:\n No delivery");
      digitalWrite(ERROR_LED, HIGH);
      break;
    case TICK_UPDATE_TIMEOUT:
      disp.write("Update:\n Timeout");
      digitalWrite(ERROR_LED, HIGH);
      break;
    case TICK_NEW_MSG:
      disp.write((char *) radio.getData());
      debug("GOT NEW MSG");
      break;
    case TICK_SLEEPING:
      // Radio is sleeping and we have nothing to do; lets sleep!
      {
        uint8_t sleepRounds = 3;
        // Sleep until we've sleept long enough or a button has been pressed.
        while (sleepRounds-- && !joinPressed && !updatePressed) {
          sleep();
        }
      }
      // If the join button was pressed we shouldn't update the message.
      if (!joinPressed) {
        radio.wakeup();
        radio.getNewestMessage();
      }
      updatePressed = false;
      break;
    case TICK_UNKNOWN_ERROR:
      digitalWrite(ERROR_LED, HIGH);
      break;
    case TICK_JOIN_BAD_MSG:
      digitalWrite(ERROR_LED, HIGH);
      break;
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

