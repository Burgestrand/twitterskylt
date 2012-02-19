#include "arduino_helpers.h"
#include <stdio.h>

void arduino_printf(char *fmt, ... )
{
  char tmp[1024]; // resulting string limited to 1024 chars
  va_list args;
  va_start(args, fmt );
  vsnprintf(tmp, 1024, fmt, args);
  va_end(args);
  Serial.println(tmp);
}
