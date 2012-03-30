#include "HTTP.h"

namespace HTTP
{
	static boolean done = false;
	
	uint8_t tick(Coordinator * coordinator)
	{
		if (!done)
		{
			char tweet[] = "charlie sheen ro\nxx";
			char twitter_date[] = "Fri, 30 Mar 2012 23:23:07 +0000";
			int utc_offset = 3600;
			char * message = Formatting::format(tweet, twitter_date, utc_offset);
			coordinator->setData((uint8_t *) tweet, (uint8_t) strlen(tweet));
			done = true;
		}
		
		return 0;
	}
}
