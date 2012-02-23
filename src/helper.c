#include "helper.h"
#include <stdlib.h>
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
xmalloc(size_t size)
{
  void *ptr = malloc(size);

  if (ptr == NULL)
  {
    printf("failed to allocate memory\n");
    abort();
  }

  memset(ptr, 0, size);
  return ptr;
}
