#include "util.h"

unsigned char *justify(unsigned char **words, int total_length)
{
  unsigned char *result = ALLOC_USTR(160);
  unsigned char *cursor = result;
  unsigned char **original_words = words;
  int i = 0;

  for (i = 0; i < total_length; ++i)
  {
    unsigned char *word = words[i];
    MEMCPY_N(cursor, "hello world", unsigned char, strlen("hello world"));
    cursor += strlen("hello world");
  }

  return result;
}
