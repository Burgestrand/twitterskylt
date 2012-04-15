#ifndef _HTTP_H_
#define _HTTP_H_

#include <stdint.h>
#include <cstring>
#include <stdarg.h>
#include "Ethernet.h"

// Callback used to report back response body
typedef void(*http_callback_t)(char *body, unsigned long length);

// Different kinds of states the HTTP client might be in
enum http_state_t
{
  HTTP_IDLE,
  HTTP_RECEIVING,
  HTTP_ERROR,
};

class HTTP
{
  public:
    HTTP(const uint8_t ip[4], size_t buffer_size, http_callback_t);
    ~HTTP();
    uint8_t get(IPAddress host, const char *path, int argc, ...);
    const uint8_t * tick(uint32_t *length);
    http_state_t state();

  private:
    EthernetClient  *_client;
    http_callback_t _callback;
    http_state_t    _state;
    uint8_t         *_buffer;
    size_t          _buffer_size;

    uint32_t        _read();
    char            *_build_query(int argc, ...);
};

#endif
