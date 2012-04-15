#include "HTTP.h"
#include "../Common/Utilities.h"
#include <stdio.h>

HTTP::HTTP(const uint8_t ip[4], size_t buffer_size, http_callback_t callback)
{
  this->_client   = new EthernetClient();
  this->_callback = callback;
  this->_buffer   = ALLOC_N(uint8_t, buffer_size);
  this->_state    = HTTP_IDLE;
}

HTTP::~HTTP()
{
  delete this->_client;
  xfree(this->_buffer);
}

uint8_t HTTP::get(IPAddress host, const char *path, int argc, ...)
{
  if ( ! (argc % 2)) // We must have an even number of arguments
  {
    return -1;
  }

  if ( ! _client->connect(host, 80))
  {
    return -2;
  }

  // Extract the query parameters
  int i = 0;
  char **query_params = ALLOC_N(char *, argc);
  va_list params;
  va_start(params, argc);
  for (i = 0; i < argc; ++i)
  {
    query_params[i] = va_arg(params, char *);
  }
  va_end(params);

  // Build the HTTP query
  char *query_string = this->_build_query(argc, query_params);
  xfree(query_params);

  // Build the GET request
  uint32_t query_length = strlen("GET ") + strlen(path) + 1 /* ? */ + strlen(query_string) + strlen(" HTTP/1.1");
  char *http_query = ALLOC_STR(query_length);
  snprintf(http_query, query_length, "GET %s?%s HTTP/1.1", path, query_string);

  // Issue the request
  this->_client->println(http_query);
  this->_client->println("HOST: api.twitter.com"); // TODO: generalize
  this->_client->println();

  // State change!
  this->_state = HTTP_RECEIVING;

  return 0;
}

char *_build_query(int argc, char **raw_query_params)
{
  // URLEncode all parameters and values
  int i = 0;
  char **query_params   = ALLOC_N(char *, argc);
  uint32_t query_length = 0;
  for (i = 0; i < argc; ++i)
  {
    query_params[i] = url_encode(raw_query_params[i]);
    query_length += strlen(query_params[i]);
    query_length += i % 2; // =
    query_length += 1; // &
  }
  query_length -= 1; // leading &

  // Construct the full query:
  char *query_string = ALLOC_STR(query_length);
  int remaining_length = query_length;

  // key=value&key=value
  for (i = 0; i < argc; ++i)
  {
    strncat(query_string, query_params[i], remaining_length);
    remaining_length -= strlen(query_params[i]);
    xfree(query_params[i]); // cleanup

    if ( ! (i % 2)) // it was a key we just added
    {
      strncat(query_string, "=", remaining_length);
      remaining_length -= 1;
    }
    else if (i >= 1 && (i + 1) != argc) // it was a value, but not last one
    {
      strncat(query_string, "&", remaining_length);
      remaining_length -= 1;
    }
  }

  xfree(query_params);

  return query_string;
}

uint32_t HTTP::_read()
{
  if (this->_client->available())
  {
    this->_client->read(this->_buffer, this->_buffer_size);
  }

  return 0;
}

http_state_t HTTP::state()
{
  return _state;
}

const uint8_t * HTTP::tick(uint32_t *length)
{
  switch (this->state())
  {
    case HTTP_IDLE:
    case HTTP_ERROR:
      *length = 0;
      return NULL;
      break;

    case HTTP_RECEIVING:
      *length = this->_read();
      return this->_buffer;
      break;
  }

  *length = -1;
  return NULL;
}

