/* Display handling code, for displaying text on the display
 */

/* TODO:
 * Inherit print class for print function
 */

#include "Arduino.h"
#include "Display.h"


Display::Display(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) : _disp(rs, e, d4, d5, d6, d7) {
//	_disp(rs, e, d4, d5, d6, d7);
	_row = 0;
	_done = 0;
}

void Display::begin(uint8_t col, uint8_t row) {
	_disp.begin(col, row);
	_disp.clear();
}

void Display::new_line() {
	_row = (_row + 1) % 2;
	_disp.setCursor(0, _row);
}

void Display::write(uint8_t c) {
	if (_done) {
		clear();
	}
	switch (c) {
		case '\004':
			_done=1;
			break;
		case '\n':
			new_line();
			_done=0;
			break;
		default:
			_disp.write(c);
			_done=0;
			break;
	}
}

void Display::clear() {
	_disp.clear();
	_row = 0;
}

