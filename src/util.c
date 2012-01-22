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
	char *result = fgets(string, size, stdin);

	if ( ! result)
  {
		return NULL;
  }
  else
  {
    char *last_char = string + strlen(string) - 1;
    if (*last_char == '\n') *last_char = '\0';
  }

	return string;
}
