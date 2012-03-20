#include <stdlib.h>
#include <string.h>

extern "C" {
	#include "yajl_parser.h"
}

namespace Parsing
{
	#define MAX_FOUND_COUNT 2
	static bool text_found = false;
	static bool date_found = false;
	static int found_count = 0;
	
	// These store the result of the parse but should really be passed as pointers in the context argument if possible.
	static char *text; 
	static char *twitter_date; 

	static int message_key_event(void *context, const unsigned char *key, size_t key_length)
	{
		if (!strncmp("text", (const char *) key, key_length)) {
			text_found = true;
		} else if (!strncmp("created_at", (const char *) key, key_length)) {
			date_found = true;
		}
		
		return 1;
	}

	static int message_string_event(void *context, const unsigned char *str, size_t str_length)
	{
		if (text_found) {
			text = (char*) malloc(str_length + 1);
			memcpy(text, str, str_length);
			text[str_length] = '\0';
			text_found = false;
			++found_count;
		} else if (date_found) {
			twitter_date = (char*) malloc(str_length + 1);
			memcpy(twitter_date, str, str_length);
			twitter_date[str_length] = '\0';
			date_found = false;
			++found_count;
		}
		
		return found_count != MAX_FOUND_COUNT;
	}

	static yajl_callbacks message_callbacks =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		message_string_event,
		NULL,
		message_key_event,
		NULL,
		NULL,
		NULL
	};

	bool parse_message(char *json, char **result_text, char **result_twitter_date)
	{
		yajl_handle handle = yajl_alloc(&message_callbacks, NULL, NULL);
		yajl_status status = yajl_parse(handle, (const unsigned char *) json, strlen(json));
		
		if (status == yajl_status_ok)
		{
			*result_text = text;
			*result_twitter_date = twitter_date;
			return true;
		}
		else
		{
			return false;
		}
	}
	
	static bool offset_found = false;
	static int offset;

	static int user_integer_event(void *context, long long number)
	{
		if (offset_found)
		{
			offset = (int) number;
			offset_found = false;
			return 0;
		}
		else
		{
			return 1;
		}
	}

	static int user_key_event(void *context, const unsigned char *key, size_t key_length)
	{
		if (!strncmp("utc_offset", (const char *) key, key_length)) {
			offset_found = true;
		}
		
		return 1;
	}

	static yajl_callbacks user_callbacks =
	{
		NULL,
		NULL,
		user_integer_event,
		NULL,
		NULL,
		NULL,
		NULL,
		user_key_event,
		NULL,
		NULL,
		NULL
	};
	
	bool parse_user(char *json, int *result_offset)
	{
		yajl_handle handle = yajl_alloc(&user_callbacks, NULL, NULL);
		yajl_status status = yajl_parse(handle, (const unsigned char *) json, strlen(json));
		
		if (status == yajl_status_ok)
		{
			*result_offset = offset;
			return true;
		}
		else
		{
			return false;
		}
	}
}