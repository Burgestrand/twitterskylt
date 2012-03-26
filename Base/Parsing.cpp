#include "Parsing.h"

namespace Parsing
{
	#define MAX_FOUND_COUNT 2
	struct MessageState {
		char **text;
		bool text_found;
		char **date;
		bool date_found;
		int found_count;
	};

	static int message_key_event(void *context, const unsigned char *key, size_t key_length)
	{
		MessageState *state = (MessageState *) context;
		
		if (!strncmp("text", (const char *) key, key_length)) {
			state->text_found = true;
		} else if (!strncmp("created_at", (const char *) key, key_length)) {
			state->date_found = true;
		}
		
		return 1;
	}

	static int message_string_event(void *context, const unsigned char *str, size_t str_length)
	{
		MessageState *state = (MessageState *) context;
		
		if (state->text_found) {
			*(state->text) = (char*) malloc(str_length + 1);
			memcpy(*(state->text), str, str_length);
			(*(state->text))[str_length] = '\0';
			state->text_found = false;
			state->found_count += 1;
		} else if (state->date_found) {
			*(state->date) = (char*) malloc(str_length + 1);
			memcpy(*(state->date), str, str_length);
			(*(state->date))[str_length] = '\0';
			state->date_found = false;
			state->found_count += 1;
		}
		
		return state->found_count != MAX_FOUND_COUNT;
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

	bool parse_message(char *json, char **result_text, char **result_date)
	{
		MessageState state = {};
		state.text = result_text;
		state.date = result_date;
		
		yajl_handle handle = yajl_alloc(&message_callbacks, NULL, &state);
		yajl_status status = yajl_parse(handle, (const unsigned char *) json, strlen(json));
		
		return status == yajl_status_ok;
	}
	
	struct UserState {
		bool offset_found;
		int *offset;
	}

	static int user_integer_event(void *context, long long number)
	{
		UserState *state = (UserState *) context;
		
		if (state->offset_found)
		{
			*(state->offset) = (int) number;
			state->offset_found = false;
			return 0;
		}
		else
		{
			return 1;
		}
	}

	static int user_key_event(void *context, const unsigned char *key, size_t key_length)
	{
		UserState *state = (UserState *) context;
		
		if (!strncmp("utc_offset", (const char *) key, key_length)) {
			state->offset_found = true;
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
		UserState state = {};
		state.offset_found = false;
		state.offset = result_offset;
		
		yajl_handle handle = yajl_alloc(&user_callbacks, NULL, &state);
		yajl_status status = yajl_parse(handle, (const unsigned char *) json, strlen(json));
		
		return status == yajl_status_ok;
	}
}