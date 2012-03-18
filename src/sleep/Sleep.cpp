#include "Sleep.h"

Sleep::Sleep() {

}

void Sleep::begin(uint8_t sleepSeconds) {
	// Store how many 8 second intervals we should sleep
	this->sleepSeconds = sleepSeconds;
	// Set sleep mode to Power Save
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	// Clear delay counter
	timerSeconds = 0;
	// Disable global interrupts
	noInterrupts();
	
	// Initialize watchdog
	// Reset watch dog reset flag
	MCUSR &= ~(1<<WDRF);
	// Configure watchdog to be configurable
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	// Set prescaler
	WDTCSR = (1<<WDCE) | (1<<WDP0) | (1<<WDP3);

	// Enable all interrupts
	interrupts();
}

// Act on interrupt from Watchdog
bool Sleep::shouldWakeUp() {
	timerSeconds++;
	if (timerSeconds == sleepSeconds) {
		timerSeconds = 0;
		return true;
	}
	return false;
}

// Go to power save mode
void Sleep::sleep() {
	noInterrupts();
	// Enable watchdog interrupts
	WDTCSR |= (1<<WDIE);
	interrupts();

	sleep_enable();
	sleep_mode();
	sleep_disable();

	noInterrupts();
	// Disable watchdog interrupts
	WDTCSR &= ~(1<<WDIE);
	interrupts();
}
