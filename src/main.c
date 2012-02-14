#include "cleaning.h"
#include "util.h"
#include "twitter.h"

int
main(void)
{
  tw_tweet *tweet = ALLOC_STR(TW_SIZE_TWEET);

  printf("Tweet: ");
  tweet = getstr(tweet, TW_SIZE_TWEET);

  /* Print it */
  printf("(%ld) %s\n", strlen(tweet), utf8_strip(tweet));

  return 0;
}
