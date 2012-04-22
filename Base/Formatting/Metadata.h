#ifndef _METADATA_H_
#define _METADATA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Time.h"
#include "Constants.h"

namespace Formatting
{
	char *add_date_placeholder(char *message);
	char *convert_date(char *twitter_date, int utc_offset);
	char *add_date(char *message, char *date);
}

#endif
