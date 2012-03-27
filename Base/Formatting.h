#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Time/Time.h"

namespace Formatting
{
	char *justify(char **original_words, uint8_t word_count);
	char *convert_date(char *twitter_date, int utc_offset);
	char *add_date(char *message, char *date);
}