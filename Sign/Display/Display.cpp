#include "Display.h"

// TODO: Should be in external file, with rest of the characters.
static const uint8_t font[] PROGMEM = {
	/* 0 0x00 '^@' */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */

	/* 1 0x01 '^A' */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x3f, 0x80, /* 0011111110 */
	0x40, 0x40, /* 0100000001 */
	0x5b, 0x40, /* 0101101101 */
	0x40, 0x40, /* 0100000001 */
	0x44, 0x40, /* 0100010001 */
	0x44, 0x40, /* 0100010001 */
	0x51, 0x40, /* 0101000101 */
	0x4e, 0x40, /* 0100111001 */
	0x40, 0x40, /* 0100000001 */
	0x3f, 0x80, /* 0011111110 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */

	/* 2 0x02 '^B' */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x3f, 0x80, /* 0011111110 */
	0x7f, 0xc0, /* 0111111111 */
	0x64, 0xc0, /* 0110010011 */
	0x7f, 0xc0, /* 0111111111 */
	0x7b, 0xc0, /* 0111101111 */
	0x7b, 0xc0, /* 0111101111 */
	0x6e, 0xc0, /* 0110111011 */
	0x71, 0xc0, /* 0111000111 */
	0x7f, 0xc0, /* 0111111111 */
	0x3f, 0x80, /* 0011111110 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */

	/* 3 0x03 '^C' */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x11, 0x00, /* 0001000100 */
	0x3b, 0x80, /* 0011101110 */
	0x7f, 0xc0, /* 0111111111 */
	0x3f, 0x80, /* 0011111110 */
	0x3f, 0x80, /* 0011111110 */
	0x1f, 0x00, /* 0001111100 */
	0x1f, 0x00, /* 0001111100 */
	0x0e, 0x00, /* 0000111000 */
	0x0e, 0x00, /* 0000111000 */
	0x04, 0x00, /* 0000010000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
	0x00, 0x00, /* 0000000000 */
};

#define CMD_WRITE 0x00
#define CMD_UPDFULL 0x18
#define CMD_UPDATE_FULL 0x18
#define CMD_FILL 0x01
#define CMD_DARK 0x12
#define CMD_SLEEP 0x20

#define ROWS_PER_LINE 18
#define BYTES_PER_ROW 30
#define FONT_BYTES_PER_ROW 2
#define FONT_BYTES_PER_CHAR 36

Display::Display(uint8_t cs_pin, uint8_t bsy_pin, uint8_t rst_pin) :
	cselect_pin(cs_pin), busy_pin(bsy_pin), reset_pin(rst_pin) {
}

void Display::begin() {
	// Configure the SPI interface.
	SPI.begin();
	SPI.setDataMode(SPI_MODE1);
	SPI.setClockDivider(SPI_CLOCK_DIV64);
	SPI.setBitOrder(MSBFIRST);

	// Configure the pins.
	pinMode(reset_pin, INPUT);
	pinMode(reset_pin, OUTPUT);

	// Make sure CS is high.
	digitalWrite(cselect_pin, HIGH);

	// Reset the display.
	reset();

	// Set the display to all black.
	black();

	// Set the display to sleep mode to preserve THE POWER!
	sleep();
}

void Display::write(char *msg) {
	clear_buffer();

	uint8_t line = 0;                      // The line number of the line being written to the display.
	uint8_t *line_start = (uint8_t *) msg; // The start of the line being written to the display.
	uint8_t *line_chr;                     // The character being written to the display.

	while (true) {
		uint8_t row = 0;                   // The pixel row on the line we're going to render.
		// Write each row to the display.
		while (row < ROWS_PER_LINE) {
			line_chr = line_start;
			write_start(line, row);

			// Write each character to the display, until row or message ends.
			while (*line_chr != 0 && *line_chr != '\n') {
				// Characters has to be sent in pairs, as they are 1.5 bytes wide.
				uint8_t buf[] = {0, 0, 0};
				bool two_chars = false;

				// Buffer the first character.
				buf[0] = font_data(*line_chr - 1, row, 0);
				buf[1] = font_data(*line_chr - 1, row, 1);

				// Buffer the second character if there is one.
				line_chr++;
				if (*line_chr != 0 && *line_chr != '\n') {
					two_chars = true;

					buf[1] |= font_data(*line_chr - 1, row, 0) >> 4;
					buf[2]  = font_data(*line_chr - 1, row, 0) << 4;
					buf[2] |= font_data(*line_chr - 1, row, 1) >> 4;

					line_chr++;
				}

				// Send all valid buffer data to the display.
				send(buf[0]);
				send(buf[1]);
				if (two_chars) {
					send(buf[2]);
				}
			}
			row++;
			end_cmd();
		}

		line++;

		if (*line_chr == 0) {
			// All rows of the message has been printed, quit.
			break;
		} else {
			// Skip over the new line character.
			line_start = line_chr + 1;
		}
	}

	// Update the display.
	update();

	// Set the display to sleep mode to preserve THE POWER!
	sleep();
}

void Display::clear_buffer() {
	start_cmd(CMD_FILL);
	send(0);   // High byte of start of area
	send(0);   // Low byte
	send(18);  // High byte of end of area (End = bytes per row*rows = 30*160 = 4800).
	send(192); // Low byte
	send(0);   // Data to fill with.
	end_cmd();
}

void Display::reset() {
	digitalWrite(reset_pin, LOW);
	digitalWrite(reset_pin, HIGH);
	wait();
}

void Display::black() {
	short_cmd(CMD_DARK);
}

void Display::write_start(uint8_t line, uint8_t row) {
	start_cmd(CMD_WRITE);
	uint16_t adr = BYTES_PER_ROW*(row + line*(ROWS_PER_LINE+1)); // +1 for line spacing
	send((uint8_t) (adr >> 8));
	send((uint8_t) adr);
}

void Display::write_data(uint8_t data) {
	send(data);
}

void Display::write_end() {
	end_cmd();
}

void Display::update() {
	start_cmd(CMD_UPDATE_FULL);
	send(0); // High byte of start of image buffer data to be displayed.
	send(0); // Low byte.
	end_cmd();
}

void Display::sleep() {
	short_cmd(CMD_SLEEP);
}

void Display::start_cmd(uint8_t cmd) {
	digitalWrite(cselect_pin, LOW);
	send(cmd);
}

void Display::end_cmd() {
	digitalWrite(cselect_pin, HIGH);
	wait();
}

void Display::short_cmd(uint8_t cmd) {
	start_cmd(cmd);
	end_cmd();
}

void Display::wait() {
	while (digitalRead(busy_pin)) { }
}

void Display::send(uint8_t data) {
	SPI.transfer(data);
}

uint8_t Display::font_data(uint8_t chr, uint8_t row, uint8_t byte) {
	return pgm_read_byte_near(font + chr*FONT_BYTES_PER_CHAR + row*FONT_BYTES_PER_ROW + byte);
}

