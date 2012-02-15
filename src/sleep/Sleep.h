#ifndef Sleep_h
#define Sleep_h

#define SLEEPSECONDS 1

// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <Arduino.h>

class Sleep {
	
	public:
		// Default constructor
		Sleep();
		void begin(uint8_t sleepSeconds = SLEEPSECONDS);
		bool shouldWakeUp();
		void sleep();
	private:
		volatile uint16_t timerSeconds;
		uint8_t sleepSeconds;
};

#endif
