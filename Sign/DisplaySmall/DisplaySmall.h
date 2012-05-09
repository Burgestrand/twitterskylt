/* Display handling code, for displaying text on the display
 *
 * Create display instance with enable, rs and data pins
 * use write/clear to display data on the display
 */

#ifndef DisplaySmall_h
#define DisplaySmall_h

#include "Arduino.h"
#include <LiquidCrystal.h>

class DisplaySmall {
	public:
		// Define the pins used with the display.
		Display(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

		// Initialize and clear the display.
		void begin(uint8_t col, uint8_t row);

		// Write a char to the display. Newlines are handled.
		void write(uint8_t c);

		// Writes each charater to the display by repeatedly calling write(uint8_t) until a null
		// byte is encountered.
		void write(char * msg);
	private:
		// Set the cursor the the start of the next line, wraps around to the top line if needed.
		void new_line();

		// Clear the display.
		void clear();

		// The library used to communicate with the display.
		LiquidCrystal _disp;

		// Current row.
		uint8_t _row;
};

#endif
