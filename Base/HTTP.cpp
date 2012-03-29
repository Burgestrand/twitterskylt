#include <string>
#include <string.h>

#ifndef ARDUINO
using namespace std;
#endif

namespace HTTP
{
	#ifdef ARDUINO
	static String json = "{\"completed_in\":0.022,\"max_id\":185305354622611457,\"max_id_str\":\"185305354622611457\",\"next_page\":\"?page=2&max_id=185305354622611457&q=from%3Aajstream%20%23egypt&rpp=1\",\"page\":1,\"query\":\"from%3Aajstream+%23egypt\",\"refresh_url\":\"?since_id=185305354622611457&q=from%3Aajstream%20%23egypt\",\"results\":[{\"created_at\":\"Thu, 29 Mar 2012 10:00:14 +0000\",\"from_user\":\"AJStream\",\"from_user_id\":236891946,\"from_user_id_str\":\"236891946\",\"from_user_name\":\"The Stream\",\"geo\":null,\"id\":185305354622611457,\"id_str\":\"185305354622611457\",\"iso_language_code\":\"en\",\"metadata\":{\"result_type\":\"recent\"},\"profile_image_url\":\"http:\\/\\/a0.twimg.com\\/profile_images\\/1704309127\\/s_normal.gif\",\"profile_image_url_https\":\"https:\\/\\/si0.twimg.com\\/profile_images\\/1704309127\\/s_normal.gif\",\"source\":\"&lt;a href=&quot;http:\\/\\/www.hootsuite.com&quot; rel=&quot;nofollow&quot;&gt;HootSuite&lt;\\/a&gt;\",\"text\":\"How does #Tunisia's democratic transition compare to that of #Egypt? - http:\\/\\/t.co\\/GqDUx75c\",\"to_user\":null,\"to_user_id\":null,\"to_user_id_str\":null,\"to_user_name\":null}],\"results_per_page\":1,\"since_id\":0,\"since_id_str\":\"0\"}";
	#else
	static string json = "{\"completed_in\":0.022,\"max_id\":185305354622611457,\"max_id_str\":\"185305354622611457\",\"next_page\":\"?page=2&max_id=185305354622611457&q=from%3Aajstream%20%23egypt&rpp=1\",\"page\":1,\"query\":\"from%3Aajstream+%23egypt\",\"refresh_url\":\"?since_id=185305354622611457&q=from%3Aajstream%20%23egypt\",\"results\":[{\"created_at\":\"Thu, 29 Mar 2012 10:00:14 +0000\",\"from_user\":\"AJStream\",\"from_user_id\":236891946,\"from_user_id_str\":\"236891946\",\"from_user_name\":\"The Stream\",\"geo\":null,\"id\":185305354622611457,\"id_str\":\"185305354622611457\",\"iso_language_code\":\"en\",\"metadata\":{\"result_type\":\"recent\"},\"profile_image_url\":\"http:\\/\\/a0.twimg.com\\/profile_images\\/1704309127\\/s_normal.gif\",\"profile_image_url_https\":\"https:\\/\\/si0.twimg.com\\/profile_images\\/1704309127\\/s_normal.gif\",\"source\":\"&lt;a href=&quot;http:\\/\\/www.hootsuite.com&quot; rel=&quot;nofollow&quot;&gt;HootSuite&lt;\\/a&gt;\",\"text\":\"How does #Tunisia's democratic transition compare to that of #Egypt? - http:\\/\\/t.co\\/GqDUx75c\",\"to_user\":null,\"to_user_id\":null,\"to_user_id_str\":null,\"to_user_name\":null}],\"results_per_page\":1,\"since_id\":0,\"since_id_str\":\"0\"}";
	#endif
	
	#ifdef ARDUINO
	const char *message()
	{
		int key_offset = json.indexOf("\"text\":\"");
		int start_message_offset = key_offset + 8;
		int end_quote_offset = json.find("\"", start_message_offset);
		int length = end_quote_offset - start_message_offset;
		String message = json.substring(start_message_offset, length);
		
		char message_c[160];
		message.toCharArray(message_c, 160);
		return message_c;
	}
	#else
	const char *message()
	{
		int key_offset = json.find("\"text\":\"");
		int start_message_offset = key_offset + 8;
		int end_quote_offset = json.find("\"", start_message_offset);
		int length = end_quote_offset - start_message_offset;
		string message = json.substr(start_message_offset, length);
		
		return message.c_str();
	}
	#endif
	
	#ifdef ARDUINO
	const char *date()
	{
		int key_offset = json.indexOf("\"created_at\":\"");
		int start_date_offset = key_offset + 14;
		int end_quote_offset = json.find("\"", start_date_offset);
		int length = end_quote_offset - start_date_offset;
		String message = json.substring(start_date_offset, length);
		
		char message_c[31];
		message.toCharArray(message_c, 31);
		return message_c;
	}
	#else
	const char *date()
	{
		int key_offset = json.find("\"created_at\":\"");
		int start_date_offset = key_offset + 14;
		int end_quote_offset = json.find("\"", start_date_offset);
		int length = end_quote_offset - start_date_offset;
		string message = json.substr(start_date_offset, length);
		
		return message.c_str();
	}
	#endif
}

#ifndef ARDUINO
#include <stdio.h>
int main(void)
{
	printf("Message: %s\n", HTTP::message());
	printf("Date:    %s\n", HTTP::date());
}
#endif