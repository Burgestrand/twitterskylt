#ifndef _HTTP_H_
#define _HTTP_H_

#include "Formatting.h"
#include "Arduino.h"
#include "Coordinator.h"

namespace HTTP
{
	uint8_t tick(Coordinator * coordinator);
}

#endif
