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
		UserParser(int *utcOffset);
		bool parse(const char *buffer, int bufferSize);

		struct State {
			int *utcOffset;
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
