#include "helper.h"
#include "splitting.h"

static int matches(char byte, char *tokens)
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

char **strsplit(const char *_string, int *num_words)
{
  char **words = NULL;
  char *string = strclone(_string);
  char *cursor = string;
  char *tokens = " ";

  if (_string == NULL)
  {
    return words;
  }

  /* first, we count number of words */
  *num_words = 0;
  int on_word  = false;
  int on_whitespace = false;

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
  words  = ALLOC_N(char *, *num_words);
  cursor = strtok(string, tokens);
  int i  = 0;

  while (cursor != NULL)
  {
    words[i++] = strclone(cursor);
    cursor = strtok(NULL, tokens);
  }

  /* free resources */
  xfree(string);

  return words;
}
