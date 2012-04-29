#ifndef _HTTP_H_
#define _HTTP_H_

#ifndef ARDUINO
#include <cstring>
#else
#include <EthernetClient.h>
#include <IPAddress.h>
#endif

#include <stdint.h>
#include <stdarg.h>

// Different kinds of states the HTTP client might be in
enum http_state_t
{
  HTTP_IDLE,
  HTTP_RECEIVING,
  HTTP_READING_BODY,
  HTTP_ERROR,
  HTTP_DONE,
};

class HTTP
{
  public:
    HTTP(const char *http_host, size_t buffer_size);
    void destroy();
    int8_t get(IPAddress host, const char *path, int argc, ...);
    const char *tick(int32_t *length);
    http_state_t state();
    const char *body();
    void body(const char *data, size_t length);

  private:
    EthernetClient  *_client;
    http_state_t    _state;
    uint8_t         *_buffer;
    size_t          _buffer_size;
    char            *_body;
    const char      *_body_cursor;
    const char      *_http_host;

    uint32_t        _read();
    char            *_build_query(int argc, char **raw_query_params);
};

#endif
