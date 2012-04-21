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
  const char *position = NULL;

  // if all bytes need encoding, the string will grow with a maximum of three
  // times its own size
  char *result = ALLOC_N(char, strlen(str) * 3 + 1);

  for (position = str; current = *position; position++) // let’s hope all our urlencoded strings ends with \0
  {
    if (isalnum(current) || current == '-' || current == '_' || current == '.' || current == '~')
    {
      *result++ = current;
    }
    else if (current == ' ')
    {
      *result++ = '+';
    }
    else
    {
      *result++ = '%';
      *result++ = to_hex(current >> 4);
      *result++ = to_hex(current & 0x0F);
    }
  }

  *result = '\0';

  return result;
}
