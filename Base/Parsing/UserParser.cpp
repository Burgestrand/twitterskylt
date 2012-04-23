#include "TweetParser.h"

static int keyEvent(void *context, const unsigned char *key, size_t keyLength)
{
	UserParser::State *state = (UserParser::State *) context;

	if (!strncmp("utc_offset", (const char *) key, keyLength)) {
		state->utcOffsetFound = true;
	}

	return 1;
}

static int integerEvent(void *context, long long number)
{
	UserParser::State *state = (UserParser::State *) context;

	if (state->utcOffsetFound)
	{
		*(state->utcOffset) = (int) number;
		state->utcOffsetFound = false;
		return 0;
	}
	else
	{
		return 1;
	}
}

TweetParser::TweetParser(char *buffer, char *utcOffset, int utcOffsetLength)
{
	this->buffer = buffer;
	
	this->state.utcOffset = utcOffset;
	this->state.utcOffsetLength = utcOffsetLength;
	this->state.utcOffsetFound = false;
	
	this->callbacks.yajl_null = NULL;
	this->callbacks.yajl_boolean = NULL;
	this->callbacks.yajl_integer = integerEvent;
	this->callbacks.yajl_double = NULL;
	this->callbacks.yajl_number = NULL;
	this->callbacks.yajl_string = NULL;
	this->callbacks.yajl_start_map = NULL;
	this->callbacks.yajl_map_key = keyEvent;
	this->callbacks.yajl_end_map = NULL;
	this->callbacks.yajl_start_array = NULL;
	this->callbacks.yajl_end_array = NULL;
	
	this->handle = yajl_alloc(&this->callbacks, NULL, &this->state);
}

bool TweetParser::parse(int bufferSize)
{
	yajl_status status = yajl_parse(this->handle, (const unsigned char *) this->buffer, bufferSize);
	
	return status == yajl_status_ok;
}
