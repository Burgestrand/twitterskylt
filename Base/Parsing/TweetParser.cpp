#include "TweetParser.h"

static int keyEvent(void *context, const unsigned char *key, size_t keyLength)
{
	TweetParser::State *state = (TweetParser::State *) context;
	
	if (!strncmp("text", (const char *) key, keyLength)) {
		state->textFound = true;
	} else if (!strncmp("created_at", (const char *) key, keyLength)) {
		state->dateFound = true;
	}
	
	return 1;
}

static int stringEvent(void *context, const unsigned char *str, size_t strLength)
{
	char *copy = (char *) calloc(strLength + 1, sizeof(char));
	memcpy(copy, str, strLength);
	TweetParser::State *state = (TweetParser::State *) context;
	
	// Maybe these pointers can be returned directly instead of copying.
	// Note that they add null byte after the string.
	if (state->textFound) {
		size_t length = (strLength > state->textLength) ? state->textLength : strLength;
		
		memcpy(state->text, str, length);
		(state->text)[length] = '\0';
		state->textFound = false;
		state->foundCount += 1;
	} else if (state->dateFound) {
		size_t length = (strLength > state->dateLength) ? state->dateLength : strLength;
		
		memcpy(state->date, str, length);
		(state->date)[length] = '\0';
		state->dateFound = false;
		state->foundCount += 1;
	}
	
	return state->foundCount != MAX_FOUND_COUNT;
}

TweetParser::TweetParser(char *buffer, char *text, int textLength, char *date, int dateLength)
{
	this->buffer = buffer;
	
	this->state.text = text;
	this->state.textLength = textLength;
	this->state.date = date;
	this->state.dateLength = dateLength;
	this->state.textFound = false;
	this->state.dateFound = false;
	this->state.foundCount = 0;
	
	this->callbacks.yajl_null = NULL;
	this->callbacks.yajl_boolean = NULL;
	this->callbacks.yajl_integer = NULL;
	this->callbacks.yajl_double = NULL;
	this->callbacks.yajl_number = NULL;
	this->callbacks.yajl_string = stringEvent;
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
	
	return status == yajl_status_client_canceled;
}
