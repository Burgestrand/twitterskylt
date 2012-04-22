#include "Display.h"

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

	/* XXX: Note least significant bytes! */
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

#define DISPLAY_SELECT_PIN 10
#define DISPLAY_BUSY_PIN 8
#define DISPLAY_RESET_PIN 9

#define CMD_WRITE 0x00
#define CMD_UPDFULL 0x18
#define CMD_FILL 0x01
#define CMD_DARK 0x12
#define CMD_SLEEP 0x11

static inline void disp_wait () {
	while (digitalRead(DISPLAY_BUSY_PIN)) {}
}

Display::Display(uint8_t cs_pin, uint8_t bsy_pin, uint8_t rst_pin) :
	cselect_pin(cs_pin), busy_pin(bsy_pin), reset_pin(rst_pin)  {
		Serial.println("Construcin'");
}

void Display::begin() {
	SPI.begin();
	SPI.setDataMode(SPI_MODE1);
	SPI.setClockDivider(SPI_CLOCK_DIV64);
	SPI.setBitOrder(MSBFIRST);

	// Configure the pins.
	pinMode(DISPLAY_BUSY_PIN, INPUT);
	pinMode(DISPLAY_RESET_PIN, OUTPUT);

	// Make sure CS is high.
	digitalWrite(DISPLAY_SELECT_PIN, HIGH);

	Serial.println("Pre reset");

	// Reset the display.
	digitalWrite(DISPLAY_RESET_PIN, LOW);
	digitalWrite(DISPLAY_RESET_PIN, HIGH);
	disp_wait();

	Serial.println("Post reset");

	// Set the display to all black.
	digitalWrite(DISPLAY_SELECT_PIN, LOW);
	SPI.transfer(CMD_DARK);
	digitalWrite(DISPLAY_SELECT_PIN, HIGH);
	disp_wait();

	Serial.println("Done initializing");
}

void Display::write(char *msgAAA) {
	Serial.println("Writin'");
	// Clear ram
	digitalWrite(DISPLAY_SELECT_PIN, LOW);
	SPI.transfer(CMD_FILL);
	SPI.transfer(0);   // START_H
	SPI.transfer(0);   // START_L
	SPI.transfer(18);  // END_H
	SPI.transfer(192); // END_L
	SPI.transfer(0);   // DATA
	digitalWrite(DISPLAY_SELECT_PIN, HIGH);
	disp_wait();

	uint8_t msg[] = {2, 4, 3, 1, 1, 3, '\n', 2, 1, 2, 3, 3, 3, 3, '\n', 2, 3, 0};

	uint8_t line = 0; 
	uint8_t *line_start = msg;
	uint8_t *line_chr;
	while (true) {
		uint8_t row = 0;
		Serial.println("Writin' new line");
		while (row < 18) {
			line_chr = line_start;
			uint16_t adr = 30*(row + line*19);

			digitalWrite(DISPLAY_SELECT_PIN, LOW);
			SPI.transfer(CMD_WRITE);
			SPI.transfer((uint8_t) (adr >> 8));
			SPI.transfer((uint8_t) adr);

			while (*line_chr != 0 && *line_chr != '\n') {
				uint8_t buf[] = {0, 0, 0};
				bool two_chars = false;

				buf[0] = pgm_read_byte_near(font + 2*row + (*line_chr-1)*36);
				buf[1] = pgm_read_byte_near(font + 2*row + (*line_chr-1)*36 + 1);

				line_chr++;
				if (*line_chr != 0 && *line_chr != '\n') {
					two_chars = true;

					buf[1] |= pgm_read_byte_near(font + 2*row + (*line_chr-1)*36)     >> 4;
					buf[2]  = pgm_read_byte_near(font + 2*row + (*line_chr-1)*36)     << 4;
					buf[2] |= pgm_read_byte_near(font + 2*row + (*line_chr-1)*36 + 1) >> 4;

					line_chr++;
				}

				SPI.transfer(buf[0]);
				SPI.transfer(buf[1]);
				if (two_chars) {
					SPI.transfer(buf[2]);
				}
		}
			row++;

			digitalWrite(DISPLAY_SELECT_PIN, HIGH);
			disp_wait();
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

	// Update the display
	digitalWrite(DISPLAY_SELECT_PIN, LOW);
	SPI.transfer(CMD_UPDFULL);
	SPI.transfer(0);
	SPI.transfer(0);
	digitalWrite(DISPLAY_SELECT_PIN, HIGH);
	disp_wait();
	Serial.print("Done writin'");
}


void doThing() {
	// Configure SPI
	SPI.begin();
	SPI.setDataMode(SPI_MODE1);
	SPI.setClockDivider(SPI_CLOCK_DIV64);
	SPI.setBitOrder(MSBFIRST);

	// Configure Pins
	pinMode(DISPLAY_BUSY_PIN, INPUT);
	pinMode(DISPLAY_RESET_PIN, OUTPUT);

	// Make sure CS is high
	digitalWrite(DISPLAY_SELECT_PIN, HIGH);

	// Reset display
	digitalWrite(DISPLAY_RESET_PIN, LOW);
	digitalWrite(DISPLAY_RESET_PIN, HIGH);
	disp_wait();

	// Clear ram
	digitalWrite(DISPLAY_SELECT_PIN, LOW);
	SPI.transfer(CMD_FILL);
	SPI.transfer(0);   // START_H
	SPI.transfer(0);   // START_L
	SPI.transfer(18);  // END_H
	SPI.transfer(192); // END_L
	SPI.transfer(0);   // DATA
	digitalWrite(DISPLAY_SELECT_PIN, HIGH);
	disp_wait();

	uint8_t msg[] = {2, 4, 3, 1, 1, 3, '\n', 2, 1, 2, 3, 3, 3, 3, 0};

	uint8_t line = 0; 
	uint8_t *line_start = msg;
	uint8_t *line_chr;
	while (true) {
		uint8_t row = 0;
		while (row < 18) {
			line_chr = line_start;
			uint16_t adr = 30*(row + line*19);

			digitalWrite(DISPLAY_SELECT_PIN, LOW);
			SPI.transfer(CMD_WRITE);
			SPI.transfer((uint8_t) (adr >> 8));
			SPI.transfer((uint8_t) adr);

			while (*line_chr != 0 && *line_chr != '\n') {
				uint8_t buf[] = {0, 0, 0};
				bool two_chars = false;

				buf[0] = pgm_read_byte_near(font + 2*row + (*line_chr-1)*36);
				buf[1] = pgm_read_byte_near(font + 2*row + (*line_chr-1)*36 + 1);

				line_chr++;
				if (*line_chr != 0 && *line_chr != '\n') {
					two_chars = true;

					buf[1] |= pgm_read_byte_near(font + 2*row + (*line_chr-1)*36)     >> 4;
					buf[2]  = pgm_read_byte_near(font + 2*row + (*line_chr-1)*36)     << 4;
					buf[2] |= pgm_read_byte_near(font + 2*row + (*line_chr-1)*36 + 1) >> 4;

					line_chr++;
				}

				SPI.transfer(buf[0]);
				SPI.transfer(buf[1]);
				if (two_chars) {
					SPI.transfer(buf[2]);
				}
		}
			row++;

			digitalWrite(DISPLAY_SELECT_PIN, HIGH);
			disp_wait();
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

	// Update the display
	digitalWrite(DISPLAY_SELECT_PIN, LOW);
	SPI.transfer(CMD_UPDFULL);
	SPI.transfer(0);
	SPI.transfer(0);
	digitalWrite(DISPLAY_SELECT_PIN, HIGH);
	disp_wait();
}
