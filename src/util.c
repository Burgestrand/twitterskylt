#include "util.h"
#include <stdlib.h>
#include <stdio.h>

void *
xmalloc(size_t size)
{
  void *ptr = malloc(size);
  memset(ptr, 0, size);
  return ptr;
}

char *getstr(char *string, int size)
{
  // ending NULL + ending newline
  char *buffer = ALLOC_STR(size + 1);
  char *result = fgets(buffer, size + 1, stdin);

  if ( ! result)
  {
    return NULL;
  }
  else
  {
    char *newline = strchr(buffer, '\n');
    if (newline)
    {
      *newline = '\0';
    }
  }

  MEMCPY_N(string, buffer, char, strlen(buffer));
  return string;
}
