/* Module for communicating with the display */
#ifndef Display_h
#define Display_h

#include <SPI.h>
#include <avr/pgmspace.h>
//#include <Arduino.h>
#include <fontwide.h>

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

		// Reset the display.
		void reset();

		// Turn the display completely dark.
		void black();

		// Start writing a row of pixels for the row:th row of line line.
		// Clears old data from buffer.
		void write_start(uint8_t line, uint8_t row);

		// Data for the row. Don't write more than 30 bytes!
		void write_data(uint8_t data);

		// Finish writing row.
		void write_end();

		// Helpers
		// Clears image buffer.
		void clear_buffer();

		// Set display to sleep mode.
		void sleep();

		// Update the (whole) display.
		void update();

		// Start a command with arguments.
		void start_cmd(uint8_t cmd);

		// End a command with arguments.
		void end_cmd();

		// Send a command without arguments.
		void short_cmd(uint8_t cmd);

		// Wait for the display to become non-busy.
		void wait();

		// Send a byte to the display.
		void send(uint8_t data);

		// Get the offset for a row of a character.
		// byte controls whether 0:th or 1:st byte is returned.
		uint8_t font_data(uint8_t chr, uint8_t row, uint8_t byte);
};

#endif
