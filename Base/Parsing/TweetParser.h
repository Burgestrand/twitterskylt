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
		TweetParser(char *text, size_t textLength, char *date, size_t dateLength);
		bool parse(const char * buffer, int bufferSize);
		void teardown();

		struct State {
			// The variables that result values should be written into.
			char *text;
			size_t textLength;
			char *date;
			size_t dateLength;
			// Progress state.
			bool textFound;
			bool dateFound;
			uint8_t foundCount;
		};
	private:
		// Yajl.
		yajl_callbacks callbacks;
		yajl_handle handle;
		// Parser state.
		#define MAX_FOUND_COUNT 2

		TweetParser::State state;
};

#endif
