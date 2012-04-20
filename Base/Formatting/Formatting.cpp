#include "Formatting.h"

namespace Formatting
{
	char *format(char *message, char *twitter_date, int utc_offset)
	{
		char *placeholder_message = Formatting::add_date_placeholder(message);
		char *cleaned = Formatting::utf8_strip(placeholder_message);
		uint8_t word_count = 0;
		char **words = Formatting::strsplit(cleaned, &word_count);
		char *justified = Formatting::justify(words, word_count);
		char *date = Formatting::convert_date(twitter_date, utc_offset);
		char *result = Formatting::add_date(justified, date);
		
		return result;
	}
}
