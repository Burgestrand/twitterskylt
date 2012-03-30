#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Time/Time.h"
#include "Constants.h"

namespace Formatting
{
	char *convert_date(char *twitter_date, int utc_offset);
	char *add_date(char *message, char *date);
}