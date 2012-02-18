#include "Timer.h"

Timer::Timer() {
	
}

void Timer::begin() {

	noInterrupts();
	// Initalize Timer1 timeout
	TCCR1A = 0;
	TCCR1B = 0;
	// Timer match
	OCR1A = 25624;
	// Clear ontimer compare (CTC) mode
	TCCR1B |= (1 << WGM12);

	// Enable all interrupts
	interrupts();
}

void Timer::start() {
	// Reset timer counter
	TCNT1 = 0;
	// Prescaler 1024x
	TCCR1B |= (1 << CS10);
	TCCR1B |= (1 << CS12);
	// Give interrupt on timer compare
	TIMSK1 |= (1 << OCIE1A);	
}

void Timer::stop() {
	// Prescaler 1024x
	TCCR1B &= ~(1 << CS10);
	TCCR1B &= ~(1 << CS12);
	// Give interrupt on timer compare
	TIMSK1 &= ~(1 << OCIE1A);
}