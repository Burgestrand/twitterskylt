#include "Encoding.h"
#include "Normalization.h"
#include "Justification.h"
#include "Metadata.h"

namespace Formatting
{
	char *format(char *message, char *twitter_date, int utc_offset)
	{
		char *cleaned = Formatting::utf8_strip(message);
		uint8_t word_count = 0;
		char **words = Formatting::strsplit(cleaned, &word_count);
		char *justified = Formatting::justify(words, word_count);
		char *date = Formatting::convert_date(twitter_date, utc_offset);
		char *result = Formatting::add_date(justified, date);
		
		return result;
	}
}