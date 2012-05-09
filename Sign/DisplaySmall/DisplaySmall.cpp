/* Display handling code, for displaying text on the display
 */

#include <Arduino.h>
#include "DisplaySmall.h"

#define NUM_LINES 2

// Constructor defining the pins used by the display.
DisplaySmall::DisplaySmall(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) : _disp(rs, e, d4, d5, d6, d7) {
	_row = 0;
}

// Initialize and clear display.
void DisplaySmall::begin(uint8_t col, uint8_t row) {
	_disp.begin(col, row);
	_disp.clear();
}

// Set the cursor to the start of the next line, wraps around to the first line if needed.
void DisplaySmall::new_line() {
	_row = (_row + 1) % NUM_LINES;
	_disp.setCursor(0, _row);
}

// Write a char, handles newlines.
void DisplaySmall::write(uint8_t c) {
	if (c == '\n') {
		new_line();
	} else {
		_disp.write(c);
	}
}

// Null terminate the string, or else!
void DisplaySmall::write(char * msg) {
	clear();
	while (*msg != '\0') {
		write(*msg);
		msg++;
	}
}

// Clear the display.
void DisplaySmall::clear() {
	_disp.clear();
	_row = 0;
}
