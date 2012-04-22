#include "Formatting.h"

namespace Formatting
{
	char *format(char *message, char *twitter_date, int utc_offset)
	{
    char *safe_message = ALLOC_STR(140);
    MEMCPY_N(safe_message, message, char, 140);
		char *placeholder_message = Formatting::add_date_placeholder(message);

		char *cleaned = Formatting::utf8_strip(placeholder_message);
    xfree(placeholder_message);

		uint8_t word_count = 0;
		char **words = Formatting::strsplit(cleaned, &word_count);
    xfree(cleaned);

		char *justified = Formatting::justify(words, word_count);
    for (int i = 0; i < word_count; ++i)
    {
      xfree(words[i]);
    }
    xfree(words);

		char *date = Formatting::convert_date(twitter_date, utc_offset);
		char *result = Formatting::add_date(justified, date);
    xfree(date);
		
		return result;
	}
}
