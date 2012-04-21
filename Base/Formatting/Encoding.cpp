#include "Encoding.h"

namespace Formatting
{
	// maps characters to characters :)
	// charmap[actual_char] = display_char
	static unsigned char charmap[256] =
	{
		// non-printable ASCII
		// 0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    B,    C,    D,   E,     F
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x20, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0x7E
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0x10

		// printable ASCII
		// 0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    B,    C,    D,   E,     F
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 0x7E
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 0x30
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, // 0x40
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 0x50
		0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // 0x60
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7E, // 0x70

		// non-ASCII, 0x80
		// 0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    B,    C,    D,   E,     F
		0x7E, 0x7E, 0x7E, 0x7E, 0x61, 0x61, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0x80
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x6f, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0x90
		0x7E, 0x7E, 0x7E, 0x7E, 0x61, 0x61, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0xA0
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x6f, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0xB0
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0xC0
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0xD0
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0xE0
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, // 0xF0
	};

	static bool valid_utf8(unsigned char byte)
	{
		if (byte == 0xC0 || byte == 0xC1)
		{
			return false;
		}
		else if (byte >= 0xF5)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	static bool is_ascii(unsigned char byte)
	{
		return (byte & 0x80) == 0;
	}
	
	/*
	 * Takes a string of UTF8-encoded data and strips all characters we cannot use.
	 *
	 * - multibyte characters we can display are mapped over
	 * - all other multibyte characters are stripped
	 * - characters we cannot handle are turned into nulls
	 *
	 * Also see: http://en.wikipedia.org/wiki/Utf8
	 */
	char *utf8_strip(const char *dirty)
	{
		// optimistic result: we end up with the same string
		uint8_t length = strlen(dirty), ci = 0, di = 0;
		unsigned char bytes	= 0;
		unsigned char current = 0x00, next = 0x00;
		char *cleaned = ALLOC_STR(length);

		// iterate character by character and replace it
		for (di = 0, ci = 0; di < length; di++)
		{
			current = dirty[di];

			if ( ! valid_utf8(current)) // invalid byte
			{
				continue;
			}
			else if ( ! is_ascii(current)) // multibyte
			{
				if (current == 0xC3 && (di + 1) < length) // might be åäöÅÄÖ, they all are 0xC3xx
				{
					next = dirty[++di]; // we consume the next character

					if (is_ascii(next)) // somehow, next byte is ascii (invalid utf8), so abort
					{
						// we cannot safely map the next byte in our charmap as it’ll collide
						// with the ascii characters which might be bad!
						continue;
					}
					else
					{
						current = next;
					}
				}
				else // skip all the additional bytes
				{
					bytes = (current & 0xF0); // 1111 xxxx
					while (bytes <<= 1) di += 1;
					current = '\0'; // let charmap handle it
				}
			}

			cleaned[ci++] = charmap[current];
		}

		return cleaned;
	}
}
