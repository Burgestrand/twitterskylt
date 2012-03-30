#include "Justification.h"

namespace Formatting
{
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
}