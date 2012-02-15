#ifndef Sleep_h
#define Sleep_h

#define sleepSeconds 5

// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <Arduino.h>

class Sleep {
	
	public:
		// Default constructor
		Sleep();
	private:
		volatile uint16_t timerSeconds;
};

#endif
