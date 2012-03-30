namespace HTTP
{
	static done = false;
	
	uint8_t tick()
	{
		if (!done)
		{
			char tweet[] = "charlie sheen roxx";
			char twitter_date[] = "Fri, 30 Mar 2012 23:23:07 +0000";
			int utc_offset = 3600;
			char message = Formatting::format(tweet, twitter_date, utc_offset);
			Coordinator.setData(message, strlen(message));
			done = true;
		}
		
		return 0;
	}
}