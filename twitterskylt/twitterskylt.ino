#define ARDUINO
#include <aJSON.h>

#include "helper.h"

extern "C"
{
  #include "formatting.h"
  #include "splitting.h"
  #include "cleaning.h"
}

void setup()
{
  Serial.begin(9600);

  extern char *json;
  char *stripped_json = utf8_strip(json);
  aJsonObject *jsonObject = aJson.parse(stripped_json);
  aJsonObject *results    = aJson.getObjectItem(jsonObject, "results");
  aJsonObject *tweet      = aJson.getArrayItem(results, 0);

  char *tweet_text = "Hello, world!";
  Serial.println(tweet_text);
  show(tweet_text);

  int num_words = 0;
  char **words = strsplit(tweet_text, &num_words);

  char *result = justify(words, num_words);
  for (num_words--; num_words >= 0; num_words--)
  {
    free(words[num_words]);
  }
  free(words);

  show(result);
  free(result);
}

void loop()
{
}
