#ifndef TweetParser_h
#define TweetParser_h

// Constants that are not defined in AVR-C but needed by Yajl.
// Most of these are just guesses, but shouldn't affect the program either way.
#define LLONG_MAX 18446744073709551615
#define LLONG_MIN 0
#define HUGE_VAL 18446744073709551615
#define ENOMEM 35
#define EINVAL 36
#define YAJL_VERSION 2

extern "C" {
	#include "yajl_parser.h"
}

class TweetParser {
	public:
		TweetParser(char *buffer, char *text, int textLength, char *date, int dateLength);
		bool parse(int bufferSize);
	private:
		// Buffer to read JSON from.
		char *buffer;
		int bufferSize;
		// Yajl.
		yajl_callbacks callbacks;
		yajl_handle handle;
		// Parser state.
		#define MAX_FOUND_COUNT 2
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
		State state;
};

#endif
