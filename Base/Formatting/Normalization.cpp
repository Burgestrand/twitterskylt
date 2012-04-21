#include "Normalization.h"
#include "Utilities.h"

namespace Formatting
{
	static char *strclone(const char *string)
	{
		char *clone = NULL;

		if (string == NULL)
		{
			return NULL;
		}

		clone = ALLOC_STR(strlen(string));
		strcpy(clone, string);
		return clone;
	}
	
	static bool matches(char byte, char *tokens)
	{
		char *token = NULL;

		for (token = tokens; *token != '\0'; ++token)
		{
			if (*token == byte)
			{
				return true;
			}
		}

		return false;
	}

	char **strsplit(const char *_string, uint8_t *num_words)
	{
		char **words = NULL;
		char *string = strclone(_string);
		char *cursor = string;
		char tokens[] = " ";

		if (_string == NULL)
		{
			return words;
		}

		/* first, we count number of words */
		*num_words = 0;
		bool on_word	= false;
		bool on_whitespace = false;

		for (cursor = string; *cursor != '\0'; ++cursor)
		{
			on_whitespace = matches(*cursor, tokens);

			if (on_whitespace && on_word)
			{
				on_word = false;
			}
			else if ( ! on_whitespace && ! on_word)
			{
				(*num_words)++;
				on_word = true;
			}
		}

		/* allocate space for all words */
		words	= ALLOC_N(char *, *num_words);
		cursor = strtok(string, tokens);
		uint8_t i	= 0;

		while (cursor != NULL)
		{
			words[i++] = strclone(cursor);
			cursor = strtok(NULL, tokens);
		}

		/* free resources */
		xfree(string);

		return words;
	}
}
