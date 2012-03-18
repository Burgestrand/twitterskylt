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
		Display(uint8_t rs, uint8_t e, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
		void begin(uint8_t col, uint8_t row);
		void new_line();
		void clear();
		void write(uint8_t c);
		void write(String str);
	private:
		LiquidCrystal _disp;
		uint8_t _row;
		uint8_t _done;
};

#endif
