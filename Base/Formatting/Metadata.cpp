#include "Metadata.h"

namespace Formatting
{
	char *add_date_placeholder(char *message)
	{
		const char *placeholder = " 0123456789abc";
		uint8_t length_after = strlen(message) + strlen(placeholder) + 1;
		char * placeholder_message = (char *) realloc(message, length_after);
		strcat(placeholder_message, placeholder);
		return placeholder_message;
	}
	
	#define DATE_FORMAT "- %u/%u %02u:%02u"
	static const char *month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	char *convert_date(char *twitter_date, int utc_offset)
	{
		// Twitter date format: Sat, 12 Jan 2012 23:34:45 +0000
		//                      0123456789012345678901234567890
		//                      0         1         2         3
		
		// Format in other parts of API: Sat Jan 12 23:34:45 +0000 2012
		// Apparently not in search API, however.
		
		// Extract year.
		unsigned int year = strtol(twitter_date + 12, NULL, 10);

		// Extract abbreviated name of month.
		char *month_name = (char *) calloc(3 + 1, sizeof(char));
		memcpy(month_name, twitter_date + 8, 3 * sizeof(char));
		
		// Calculate month's number.
		uint8_t month = 0;
		while (strcmp(month_name, month_names[month]) != 0 && month < 11) {
			++month;
		}
		++month; // Put date within interval 1-12 instead of 0-11.
		free(month_name);

		// Convert to time structure.
		TimeElements utc_date;
		utc_date.Year = year - 1970; // Offset from 1970 as required by TimeElements.
		utc_date.Month = month;
		utc_date.Day = strtol(twitter_date + 5, NULL, 10); // Extract day.
		utc_date.Hour = strtol(twitter_date + 17, NULL, 10); // Extract hour.
		utc_date.Minute = strtol(twitter_date + 20, NULL, 10); // Extract minute.
		utc_date.Second = 0;

		// Convert to timestamp and apply UTC offset.
		time_t utc_time = makeTime(utc_date);
		time_t other_time = utc_time + utc_offset;

		// Convert back into time structure.
		TimeElements other_date;
		breakTime(other_time, other_date);

		// Length of minimal format string:
		// "- d/m hh:mm"
		//  12345678901
		uint8_t result_length = 11;
		
		// Consider days and months with two digits.
		if (other_date.Day >= 10) { ++result_length; }
		if (other_date.Month >= 10) { ++result_length; }

		// Create the final string.
		char *result = (char *) calloc(result_length + 1, sizeof(char));
		sprintf(result, DATE_FORMAT, other_date.Day, other_date.Month, other_date.Hour, other_date.Minute);

		return result;
	}

	char *add_date(char *message, char *date)
	{
		uint8_t date_length = strlen(date);
		char *p = message;
		
		// Count the number of newlines.
		uint8_t newline_count = 0;
		for (; *p != '\0'; p++)
		{
			if (*p == '\n')
			{
				++newline_count;
			}
		}
		
		// Backtrack until we find the start of the dummy date string.
		while (*p != '0')
		{
			--p;
		}
		
		// If there is a space before the dummy date string (as opposed to a newline),
		// backtrack one step because we want to remove the space as well.
		if (p != message && *(p - 1) == ' ')
		{
			--p;
		}
		
		// Insert the required number of newlines (possibly zero).
		uint8_t newlines_needed = (LINE_COUNT - 1) - newline_count;
		for (uint8_t i = 0; i < newlines_needed; ++i)
		{
			*p = '\n';
			++p;
		}
		
		// Count the number of characters preceding the dummy date string on its line (possibly zero).
		char *q = p;
		uint8_t chars_before = 0;
		while (*q != '\n')
		{
			++chars_before;
			--q;
		}
		--chars_before; // Don't count the newline.
		
		// Pad with the required number of spaces.
		uint8_t spaces_needed = LINE_LENGTH - chars_before - date_length;
		for (uint8_t i = 0; i < spaces_needed; ++i)
		{
			*p = ' ';
			++p;
		}
		
		// Insert the date.
		memcpy(p, date, date_length * sizeof(char));
		
		return message;
	}
}
