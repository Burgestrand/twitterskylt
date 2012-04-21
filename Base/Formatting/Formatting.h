#ifndef _FORMATTING_H_
#define _FORMATTING_H_

#include <inttypes.h>
#include <Utilities.h>
#include "Encoding.h"
#include "Normalization.h"
#include "Justification.h"
#include "Metadata.h"

namespace Formatting
{
	/*
	Apply all formatting to a message so that it can be shown on a display:
	
	- encoding (remove invalid characters)
	- normalization (collapse whitespace)
	- justification (distribute words over lines on display)
	- metadata (add date to end of string)
	
	*/
	char *format(char *message, char *twitter_date, int utc_offset);
}

#endif
