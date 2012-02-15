#include "Sleep.h"

Sleep::Sleep() {

	// Set sleep mode to Power Save
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	// Clear delay counter
	timerSeconds = 0;
    // Initialize Timer1
	// Disable global interrupts
    cli();          
    TCCR1A = 0;
    TCCR1B = 0;
    // Timer match
    OCR1A = 15624;
    // Clear on timer compare (CTC) mode
    TCCR1B	 |= (1 << WGM12);
    // Prescaler 1024x
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    // Give interrupt on timer compare
    TIMSK1 |= (1 << OCIE1A);
    // Enable global interrupts
    sei();
}
 
// Act on interrupt from Timer1
bool Sleep::shouldWakeUp() {
   	timerSeconds++;
    if (timerSeconds == sleepSeconds) {
        timerSeconds = 0;
        return true;
    }
    return false;
}

void Sleep::sleep() {
	
}

void Sleep::unsleep() {
	
}
