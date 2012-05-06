#include "Utilities.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

char *
strclone(const char *string)
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

void
xfree(void *ptr)
{
  if (ptr != NULL)
  {
    free(ptr);
  }
}

void *
xcalloc(size_t count, size_t size)
{
  void *ptr = calloc(count, size);

  if (ptr == NULL)
  {
    printf("failed to allocate memory\n");
    abort();
  }

  return ptr;
}

int
find(const char *haystack, size_t length, const char *needle, const char **cursor)
{
  for (unsigned int i = 0; i < length; ++i)
  {
    if (**cursor == haystack[i])
    {
      ++(*cursor);
      if (**cursor == '\0')
        return i;
    }
    else if (*cursor != needle)
    {
      *cursor = needle;
      i -= 1;
    }
    else
    {
       // do nothing, or risk endless loop!
    }
  }

  return -1;
}

/*
 * Tagen från https://github.com/Burgestrand/libmockspotify/blob/65dc038574a0ef1d82abe75abd28269fb05a0c66/src/urlcode.c
 * */

static char to_hex(char code)
{
  static char hex[] = "0123456789ABCDEF";
  return hex[code & 0x0F];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(const char *str)
{
  char current;
  const char *cursor = NULL;

  // if all bytes need encoding, the string will grow with a maximum of three
  // times its own size
  char *result = ALLOC_STR(strlen(str) * 3);
  char *result_cursor = result;

  for (cursor = str; (current = *cursor); cursor++) // let’s hope all our urlencoded strings ends with \0
  {
    if (isalnum(current) || current == '-' || current == '_' || current == '.' || current == '~')
    {
      *result_cursor++ = current;
    }
    else if (current == ' ')
    {
      *result_cursor++ = '+';
    }
    else
    {
      *result_cursor++ = '%';
      *result_cursor++ = to_hex(current >> 4);
      *result_cursor++ = to_hex(current & 0x0F);
    }
  }

  *result_cursor = '\0';

  return result;
}
