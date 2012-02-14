#include "cleaning.h"
#include "util.h"
#include "twitter.h"

int
main(void)
{
  tw_username *username = ALLOC_USTR(TW_SIZE_USERNAME);
  tw_tweet    *tweet    = ALLOC_USTR(TW_SIZE_TWEET);
  tw_error error        = ERROR_NO_ERROR;

  /* Read username to fetch for */
  printf("Username: ");
  tweet = (unsigned char *) getstr((char *) username, TW_SIZE_TWEET + 1);

  /* Print it */
  printf("(%ld) %s\n", strlen(tweet), utf8_strip(tweet));

  return 0;
}
