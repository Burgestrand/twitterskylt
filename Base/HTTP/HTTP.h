#ifndef _HTTP_H_
#define _HTTP_H_

#ifndef ARDUINO
#include <cstring>
#endif

#include <EthernetClient.h>
#include <IPAddress.h>

#include <stdint.h>
#include <stdarg.h>
#include "http-parser/http_parser.h"

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
    HTTP(const uint8_t ip[4], size_t buffer_size);
    ~HTTP();
    uint8_t get(IPAddress host, const char *path, int argc, ...);
    const char *tick(uint32_t *length);
    http_state_t state();
    const char *body();
    void body(const char *data, size_t length);

  private:
    EthernetClient  *_client;
    http_state_t    _state;
    uint8_t         *_buffer;
    size_t          _buffer_size;
    char            *_body;

    http_parser     *_parser;

    uint32_t        _read();
    char            *_build_query(int argc, ...);
};

#endif
