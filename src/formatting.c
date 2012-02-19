#include "formatting.h"
#include "helper.h"

#define LINE_COUNT 4
#define LINE_LENGTH 40

static int words_length(char *words[], int word_count) {
	int result = 0;
	int i = 0;
	for (i = 0; i < word_count; ++i)
	{
		result += strlen(words[i]);
	}
	result += word_count - 1; // 1 space for each word except the last.
	
	return result;
}

char *justify(char **original_words, int word_count)
{
	char **words = ALLOC_N(char *, word_count);
	MEMCPY_N(words, original_words, char *, word_count);

	char *result = ALLOC_STR(LINE_COUNT * LINE_LENGTH + LINE_COUNT);
	char *cursor = result;
	
	int room_needed = words_length(words, word_count);
	int current_line_length = 0;
	int lines_left = LINE_COUNT - 1;
	int i = 0;
	for (i = 0; i < word_count; ++i)
	{
		char *word = words[i];
		int word_length = strlen(word);
		
		int line_chars_left = LINE_LENGTH - current_line_length;
		int room_left = lines_left * LINE_LENGTH;
		
		// If word fits on current line.
		if (word_length <= line_chars_left) {
			MEMCPY_N(cursor, word, char, word_length);
			cursor += word_length;
			current_line_length += word_length;
			room_needed -= word_length;
		}
		// If word does not fit on current line but can be moved to next line.
		else if (room_needed <= room_left && word_length <= LINE_LENGTH) {
			*cursor = '\n';
			++cursor;
			--lines_left;
			current_line_length = 0;
			
			MEMCPY_N(cursor, word, char, word_length);
			cursor += word_length;
			current_line_length += word_length;
			room_needed -= word_length;
		}
		// Otherwise, word must be broken.
		else {
			int first_part_length = line_chars_left;
			MEMCPY_N(cursor, word, char, first_part_length);
			cursor += first_part_length;
			current_line_length += first_part_length;
			room_needed -= first_part_length;
			
			// Move current string pointer and decrement word index, 
			// "tricking" the program into thinking that we've inserted a new word.
			words[i] += first_part_length;
			--i;
		}
		
		// If there are more words.
		if (i < word_count - 1) {
			// If there is room for the words on this line, add a space.
			// This means that some lines will end with a space, which should not be a problem.
			if (current_line_length < LINE_LENGTH - 1) {
				*cursor = ' ';
				++cursor;
				++current_line_length;
				--room_needed;
			}
			// Otherwise, add a newline.
			else {
				*cursor = '\n';
				++cursor;
				--lines_left;
				current_line_length = 0;
			}
		}
	}

	return result;
}

char *add_date(char *message, char *date) {
	int date_length = strlen(date);
	int newline_count = 0;
	char *p;
	for (p = message; *p != '\0'; p++) {
		if (*p == '\n') {
			++newline_count;
		}
	}
	
	// Backtrack past null byte.
	--p;
	
	// Backtrack until we find the start of the dummy date string.
	while (*p != ' ' && *p != '\n') {
		--p;
	}
	++p;
	
	// If the message occupies every line, we must pad with proper amount of spaces.
	if (newline_count == LINE_COUNT - 1) {
		// Count the number of characters on this line preceding the dummy date string.
		char *q = p;
		int line_chars = 0;
		while (*q != '\n') {
			--q;
			++line_chars;
		}
		--line_chars; // Don't count the newline.
		
		// Pad with spaces.
		int spaces_needed = LINE_LENGTH - line_chars - date_length;
		int space_index = 0;
		for (; space_index < spaces_needed; ++space_index) {
			*p = ' ';
			++p;
		}
		
		// Insert the date.
		MEMCPY_N(p, date, char, date_length);
	}
	// Otherwise we must move it down to the last line and pad with a fixed amount of spaces.
	else {
		// Insert newlines until we're on the last line.
		int newlines_needed = LINE_COUNT - newline_count - 1;
		int newline_index = 0;
		for (; newline_index < newlines_needed; ++newline_index) {
			*p = '\n';
			++p;
		}
		
		// Pad with spaces.
		int space_count = LINE_LENGTH - date_length;
		int space_index = 0;
		for (; space_index < space_count; ++space_index) {
			*p = ' ';
			++p;
		};
		
		// Insert the date.
		MEMCPY_N(p, date, char, date_length);
	}
	
	return message;
}

