#include "HTTP.h"
#include <Utilities.h>
#include <stdio.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

const char *body_divider = "\r\n\r\n";

HTTP::HTTP(const char *http_host, size_t buffer_size)
{
  this->_client   = new EthernetClient();
  this->_state    = HTTP_IDLE;
  this->_body     = NULL;
  this->_http_host = http_host;

  this->_buffer   = ALLOC_N(uint8_t, buffer_size);
  this->_buffer_size = buffer_size;
}

void HTTP::destroy()
{
  this->_client->stop();
  delete this->_client;
  xfree(this->_buffer);
}

http_error_t HTTP::get(IPAddress host, const char *path, int argc, ...)
{
  if (argc % 2) // We must have an even number of arguments
  {
    return HTTP_INVALID_ARGUMENTS;
  }

  if ( ! _client->connect(host, 80))
  {
    return HTTP_CONNECTION_FAILED;
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

  // Build the HTTP host header
  char *host_header = ALLOC_STR(strlen("HOST: ") + strlen(this->_http_host));
  sprintf(host_header, "HOST: %s", this->_http_host);

  // Build the GET request
  uint32_t query_length = strlen("GET ") + strlen(path) + 1 /* ? */ + strlen(query_string) + strlen(" HTTP/1.1") + 1 /* for null */;
  char *http_query = ALLOC_STR(query_length);
  snprintf(http_query, query_length, "GET %s?%s HTTP/1.1", path, query_string);

  // Issue the request
  Serial.println("HTTP::get http_query: ");
  Serial.println(http_query);
  Serial.println();

  this->_client->println(http_query);
  this->_client->println(host_header);
  this->_client->println("Accept: application/json");
  this->_client->println("Connection: close");
  this->_client->println(); // close stream

  // State change!
  this->_body_cursor = body_divider;
  this->_state = HTTP_RECEIVING;

  // Cleanup
  xfree(host_header);

  return HTTP_OK;
}

char *HTTP::_build_query(int argc, char **raw_query_params)
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
  #define ensure(expr) do { if ( ! (expr)) { return received; } } while(0)
  uint32_t received  = 0;
  uint32_t available = 0;
  this->body(NULL, -1); // clear previous body

  available = this->_client->available();
  Serial.print("AVAILABLE: ");
  Serial.println(available, DEC);
  Serial.println();
  ensure(available);

  uint32_t will_read = MIN(available, this->_buffer_size);
  Serial.print("WILL READ: ");
  Serial.println(will_read, DEC);
  Serial.println();

  received = this->_client->read(this->_buffer, will_read);

  Serial.print("RECEIVED: ");
  Serial.println(received, DEC);

  char *read_data = ALLOC_STR(received);
  MEMCPY_N(read_data, (const char *) this->_buffer, char, received);
  read_data[received] = '\0';
  Serial.println(read_data);
  xfree(read_data);

  ensure(received > 0);

  int found_at = -1;

  switch (this->_state)
  {
    case HTTP_RECEIVING:
      Serial.println("WAITING FOR BODY");
      found_at = find((const char *) this->_buffer, received, body_divider, &this->_body_cursor);

      if (found_at != -1)
      {
        Serial.println("FOUND BODY");
        this->_state = HTTP_READING_BODY;
        this->body((const char *) this->_buffer + found_at, received - found_at);
      }
      break;

    case HTTP_READING_BODY:
      Serial.println("READING BODY");
      this->body((const char *) this->_buffer, received);
      Serial.println("DONE READING");
      break;

    default:
      // do nothing
      break;
  }

  Serial.println("HTTP::_read DONE");

  return strlen(this->body());
}

const char *HTTP::body()
{
  return this->_body;
}

void HTTP::body(const char *data, size_t length)
{
  xfree(this->_body);
  this->_body = NULL;

  if (data and length > 0)
  {
    this->_body = ALLOC_STR(length);
    strncpy(this->_body, data, length);
  }
}

const char *HTTP::explainError(http_error_t error)
{
  switch (error)
  {
    case HTTP_OK:
      return "no error";

    case HTTP_INVALID_ARGUMENTS:
      return "invalid arguments (must be divisible by two)";

    case HTTP_CONNECTION_FAILED:
      return "connection failed";

    default:
      return "unknown error";
  }
}

http_state_t HTTP::state()
{
  return _state;
}

const char *HTTP::tick(int32_t *length)
{
  switch (this->state())
  {
    case HTTP_DONE:
      Serial.println("HTTP_DONE");
      *length = 0;
      return NULL;
      break;

    case HTTP_IDLE:
      Serial.println("HTTP_IDLE");
      *length = 0;
      return NULL;
      break;

    case HTTP_ERROR:
      Serial.println("HTTP_ERROR");
      *length = 0;
      return NULL;
      break;

    case HTTP_RECEIVING:
      Serial.println("HTTP_RECEIVING");
      *length = this->_read();
      if (*length > 0)
        return this->body();
      break;

    case HTTP_READING_BODY: // I think this is ok
      Serial.println("HTTP_READING_BODY");
      *length = this->_read();
      return this->body();
      break;

    default:
      Serial.println("HTTP_STATE UNDEFINED");
      Serial.println(this->state(), DEC);
  }

  *length = -1;
  return NULL;
}

