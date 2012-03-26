#include "Formatting.h"

namespace Formatting
{
	#define LINE_COUNT 4
	#define LINE_LENGTH 40
	#define MAX_MESSAGE_LENGTH (LINE_COUNT * LINE_LENGTH + LINE_COUNT - 1) // No newline after the last line.

	// The minimum room needed to display a list of words starting at the beginning of a line
	// and taking into account room gained by placing a word exactly at the end of a line.
	static uint8_t room_needed(char *words[], uint8_t word_count)
	{
		uint8_t result = 0;
		for (uint8_t i = 0; i < word_count; ++i)
		{
			result += strlen(words[i]);
			// Add 1 for a space unless this is the last word or the word would be placed at the end of a line.
			if (i != word_count - 1 && result % LINE_LENGTH != 0)
			{
				result += 1;
			}
		}
		
		return result;
	}

	static struct justify_state
	{
		char *cursor; // Our position in the justified string.
		uint8_t line_index; // The line that we are currently working on.
		uint8_t line_length; // How much of the current line we have currently used.
	};

	// Add a word to the current line.
	static void add_word(char *word, uint8_t length, justify_state *state)
	{
		// If there are already words on the current line, add a space.
		if (state->line_length > 0)
		{
			*(state->cursor) = ' ';
			state->cursor += 1;
			state->line_length += 1;
		}
		
		memcpy(state->cursor, word, length * sizeof(char));
		state->cursor += length;
		state->line_length += length;
	}

	// Add a newline and start working on the next line.
	static void add_newline(justify_state *state)
	{
		*(state->cursor) = '\n';
		state->cursor += 1;
		state->line_index += 1;
		state->line_length = 0;
	}

	char *justify(char **original_words, uint8_t word_count)
	{
		char **words = (char **) calloc(word_count, sizeof(char *));
		memcpy(words, original_words, word_count * sizeof(char *));

		char *result = (char *) calloc(MAX_MESSAGE_LENGTH + 1, sizeof(char));
		
		justify_state state;
		state.cursor = result;
		state.line_index = 0;
		state.line_length = 0;
		
		for (uint8_t i = 0; i < word_count; ++i)
		{
			char *word = words[i];
			uint8_t word_length = strlen(word);
			
			uint8_t words_left = word_count - i;
			uint8_t line_chars_left = LINE_LENGTH - state.line_length; 
			// If there are already words on the current line, count the space needed after them.
			if (state.line_length > 0 && line_chars_left > 0)
			{
				line_chars_left -= 1;
			}
			
			uint8_t lines_below = LINE_COUNT - (state.line_index + 1);
			uint8_t room_below = lines_below * LINE_LENGTH;
			
			// If word fits on current line.
			if (word_length <= line_chars_left) {
				add_word(word, word_length, &state);
			}
			// If word does not fit on current line but can be moved to next line.
			else if (room_needed(words + i, words_left) <= room_below && word_length <= LINE_LENGTH) {
				add_newline(&state);
				add_word(word, word_length, &state);
			}
			// Otherwise, word must be broken.
			else {
				uint8_t first_part_length = line_chars_left;
				add_word(word, first_part_length, &state);
				add_newline(&state);
				
				// Move current string pointer and decrement word index, 
				// "tricking" the program into thinking that we've inserted a new word.
				words[i] += first_part_length;
				--i;
			}
		}
		
		free(words);
		
		return result;
	}

	#define DATE_FORMAT "- %u/%u %02u:%02u"
	static const char *month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	char *convert_date(char *twitter_date, int utc_offset)
	{
		// Twitter date format: Sat Jan 12 23:34:45 +0000 2012
		//                      012345678901234567890123456789
		//                      0         1         2
		
		// Extract year.
		unsigned int year = strtol(twitter_date + 26, NULL, 10);

		// Extract abbreviated name of month.
		char *month_name = (char *) calloc(3 + 1, sizeof(char));
		memcpy(month_name, twitter_date + 4, 3 * sizeof(char));
		
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
		utc_date.Day = strtol(twitter_date + 8, NULL, 10); // Extract day.
		utc_date.Hour = strtol(twitter_date + 11, NULL, 10); // Extract hour.
		utc_date.Minute = strtol(twitter_date + 14, NULL, 10); // Extract minute.
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
		uint8_t message_length = strlen(message);
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