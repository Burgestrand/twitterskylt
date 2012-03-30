#include "Sleep.h"

/* Module for handling sleeping.
 * Call sleep_begin() in setup.
 * Call sleep() to sleep 8 seconds in low power mode (note that other timers are stopped in this mode).
 * N.B.: A watchdog interrupt handler MUST be defined, however it may be empty.
 * Example:
 *   #include <avr/interrupt.h>
 *   EMPTY_INTERRUPT(WDT_vect);
 */

// Configure the sleep mode and watchdog timer
void sleep_begin() {
	// Set sleep mode to Power Save
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	// Disable global interrupts to make sure the following code is not interrupted
	noInterrupts();

	// Configure watchdog.
	// See atmel document on the watchdog timer and device datasheet for more info.
	MCUSR  &= ~(1<<WDRF); // Reset watch dog reset flag.
	WDTCSR  =  (1<<WDCE) | (1<<WDE); // Configure watchdog to be configurable.
	WDTCSR  =  (1<<WDCE) | (1<<WDP0) | (1<<WDP3); // Set prescaler to 8 seconds.

	// Enable all interrupts.
	interrupts();
}

// Go to power save mode for at most ~8 seconds. Remember that other interrupts
// may wake the device prematurely.
void sleep() {
	noInterrupts();

	// Reset the timer.
	wdt_reset();
	// Enable watchdog interrupts.
	WDTCSR |= (1<<WDIE);

	interrupts();

	// Go to sleep.
	sleep_mode();

	noInterrupts();

	// Disable watchdog interrupts.
	WDTCSR &= ~(1<<WDIE);

	interrupts();
}
