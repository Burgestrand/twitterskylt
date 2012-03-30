#include "HTTP.h"

namespace HTTP
{
	static boolean done = false;
	
	uint8_t tick(Coordinator coordinator)
	{
		if (!done)
		{
			//char tweet[] = "charlie sheen roxx";
			//char twitter_date[] = "Fri, 30 Mar 2012 23:23:07 +0000";
			//int utc_offset = 3600;
			//uint8_t * message = (uint8_t *)Formatting::format(tweet, twitter_date, utc_offset);
			//uint8_t message[] = {'a', 'b'};
			uint8_t * testdata = NULL;
			//coordinator.setData(message, sizeof(message));
			coordinator.setData(testdata, 5);
			done = true;
		}
		
		return 0;
	}
}
