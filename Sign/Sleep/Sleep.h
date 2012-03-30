/* Module for handling sleeping.
 * Call sleep_begin() in setup.
 * Call sleep() to sleep 8 seconds in low power mode (note that other timers are stopped in this mode).
 * N.B.: A watchdog interrupt handler MUST be defined, however it may be empty.
 * Example:
 *   #include <avr/interrupt.h>
 *   EMPTY_INTERRUPT(WDT_vect);
 */

#ifndef Sleep_h
#define Sleep_h

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <Arduino.h>

// Configure the sleep mode and watchdog timer.
void sleep_begin();

// Go to power save mode for at most ~8 seconds. Other interrupts
// may wake the device prematurely.
void sleep();

#endif
