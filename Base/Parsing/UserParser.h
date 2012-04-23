#ifndef UserParser_h
#define UserParser_h

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
	#include "yajl_parser.h"
}

class UserParser {
	public:
		TweetParser(char *buffer, char *utcOffset, int utcOffsetLength);
		bool parse(int bufferSize);

		struct State {
			int *utcOffset;
			int utcOffsetLength;
			bool utcOffsetFound;
		}
	private:
		// Buffer to read JSON from.
		char *buffer;
		// Yajl.
		yajl_callbacks callbacks;
		yajl_handle handle;
		// Parser state.
		UserParser::State state;
};

#endif
