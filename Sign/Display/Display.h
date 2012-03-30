/* Display handling code, for displaying text on the display
 *
 * Create display instance with enable, rs and data pins
 * use write/newline/clear to display data on the display
 */

#ifndef Display_h
#define Display_h

#include "Arduino.h"
#include <LiquidCrystal.h>

class Display {
	public:
		// Define the pins used with the display.
		Display(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

		// Initialize and clear the display.
		void begin(uint8_t col, uint8_t row);

		// Set the cursor the the start of the next line, wraps around to the top line if needed.
		void new_line();

		// Clear the display.
		void clear();

		// Write a char to the display. Newlines are handled.
		// If a end of message character ('\004') is encountered the display is cleared before
		// the next character is written.
		void write(uint8_t c);

		// Writes each charater to the display by repeatedly calling write(uint8_t) until a null
		// byte is encountered.
		void write(char * msg);
	private:
		// The library used to communicate with the display.
		LiquidCrystal _disp;

		// Current row.
		uint8_t _row;

		// True if an end of message character has been encountered, causes the next write to
		// clear the screen before it is written.
		uint8_t _done;
};

#endif
