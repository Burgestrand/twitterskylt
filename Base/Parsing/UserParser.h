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
		UserParser(int *utcOffset, int utcOffsetLength);
		bool parse(const char *buffer, int bufferSize);

		struct State {
			int *utcOffset;
			int utcOffsetLength;
			bool utcOffsetFound;
		};
	private:
		// Yajl.
		yajl_callbacks callbacks;
		yajl_handle handle;
		// Parser state.
		UserParser::State state;
};

#endif
