#ifndef _UTIL_H_
#define _UTIL_H_

/* Required includes */
#include <string.h> /* for size_t */

#ifdef ARDUINO
#  include "arduino_helpers.h"
#  define show(fmt, ...) Serial.println(fmt)
#else
#  define show(fmt, ...) printf(fmt "\n", __VA_ARGS__)
#endif

/* Macros */
#define true 1
#define false 0

#define ALLOC(type) ALLOC_N(type, 1)
#define ALLOC_N(type, n) ((type*) xmalloc(sizeof(type) * (n)))
#define ALLOC_STR(n) ALLOC_N(char, n + 1)
#define ALLOC_USTR(n) (unsigned char *)ALLOC_STR(n)

#define MEMCPY(dst, src, type) MEMCPY_N(dst, src, type, 1)
#define MEMCPY_N(dst, src, type, n) (memcpy((dst), (src), sizeof(type) * (n)))

#ifdef DEBUG
#  define debug(fmt, ...) printf("(%s:%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);
#else
#  define debug(fmt, ...)
#endif

/* Functions */

/* like malloc, but zeroes out the memory */
void * xmalloc(size_t);

/* like free, but ignores NULL pointers */
void xfree(void *);

/* copies a string, returns a new pointer */
char * strclone(const char *);

#endif