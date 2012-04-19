#include "TweetParser.h"

static int keyEvent(void *context, const unsigned char *key, size_t keyLength)
{
	State *state = (State *) context;
	
	if (!strncmp("text", (const char *) key, keyLength)) {
		state->textFound = true;
	} else if (!strncmp("created_at", (const char *) key, keyLength)) {
		state->dateFound = true;
	}
	
	return 1;
}

static int stringEvent(void *context, const unsigned char *str, size_t strLength)
{
	State *state = (State *) context;
	
	// Maybe these pointers can be returned directly instead of copying.
	// Note that they add null byte after the string.
	if (state->textFound) {
		size_t length = (strLength > state->textLength) ? state->textLength : strLength;
		
		state->text = (char*) malloc(length + 1);
		memcpy(state->text, str, length);
		(state->text)[length] = '\0';
		state->textFound = false;
		state->foundCount += 1;
	} else if (state->dateFound) {
		size_t length = (strLength > state->dateLength) ? state->dateLength : strLength;
		
		state->date = (char*) malloc(length + 1);
		memcpy(state->date, str, length);
		(state->date)[length] = '\0';
		state->dateFound = false;
		state->foundCount += 1;
	}
	
	return state->foundCount != MAX_FOUND_COUNT;
}

TweetParser::TweetParser(char *buffer, char *text, int text_length, char *date, int date_length)
{
	this->buffer = buffer;
	
	this->state->text = text;
	this->state->text_length = text_length;
	this->state->date = date;
	this->state->date_length = date_length;
	this->state->textFound = false;
	this->state->dateFound = false;
	this->state->foundCount = 0;
	
	callbacks =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		stringEvent,
		NULL,
		keyEvent,
		NULL,
		NULL,
		NULL
	};
	handle = yajl_alloc(&callbacks, NULL, &this->state);
}

TweetParser::parse(int bufferSize)
{
	yajl_status status = yajl_parse(this->handle, (const unsigned char *) this->buffer, this->bufferSize);
	
	return status == yajl_status_ok;
}
