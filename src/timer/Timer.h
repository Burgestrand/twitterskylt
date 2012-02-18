#ifndef Timer_h
#define Timer_h

// avr-libc library includes
#include <avr/interrupt.h>

#include <Arduino.h>

class Timer {
	
	public:
		Timer();
		void begin();
		void start();
		void stop();
	private:
};

#endif
