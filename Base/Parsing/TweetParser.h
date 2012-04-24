#ifndef TweetParser_h
#define TweetParser_h

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
	#include "yajl_parser.h"
}

class TweetParser {
	public:
		TweetParser(char *buffer, char *text, int textLength, char *date, int dateLength);
		bool parse(int bufferSize);

		struct State {
			// The variables that result values should be written into.
			char *text;
			char textLength;
			char *date;
			int dateLength;
			// Progress state.
			bool textFound;
			bool dateFound;
			uint8_t foundCount;
		};
	private:
		// Buffer to read JSON from.
		char *buffer;
		// Yajl.
		yajl_callbacks callbacks;
		yajl_handle handle;
		// Parser state.
		#define MAX_FOUND_COUNT 2

		TweetParser::State state;
};

#endif