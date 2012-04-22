/* Module for communicating with the display */
#ifndef Display_h
#define Display_h

#include <SPI.h>
#include <avr/pgmspace.h>
#include <Arduino.h>

// Setup SPI interface 
void doThing();

// TODO: The rest
class Display {
	public:
		Display(uint8_t cselect_pin, uint8_t busy_pin, uint8_t reset_pin);
		void begin();
		void write(char *msg);
	protected:
	private:
		// Pins
		uint8_t cselect_pin;
		uint8_t busy_pin;
		uint8_t reset_pin;

		void reset();
		void black();
		void write_start(uint8_t line, uint8_t row);
		void write_data();
		void write_end();
		void update();
		void sleep();

		void csl();
		void csh();
		void wait();
		void send();
		void startcmd(uint8_t cmd);
};

#endif
