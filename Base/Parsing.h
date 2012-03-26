#include <stdlib.h>
#include <string.h>

extern "C" {
	#include "Yajl/yajl_parser.h"
}

namespace Parsing
{
	bool parse_message(char *json, char **result_text, char **result_twitter_date);
	bool parse_user(char *json, int *result_offset);
}