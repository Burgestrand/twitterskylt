#include "util.h"
#include "twitter.h"
#include <stdio.h>

int
main(void)
{
  unsigned char *username = ALLOC_USTR(TW_SIZE_USERNAME);
  unsigned char *tweet    = ALLOC_USTR(TW_SIZE_TWEET);
  tw_error error          = ERROR_NO_ERROR;

  /* Read username to fetch for */
  printf("Username: ");
  username = (unsigned char *) getstr((char *) username, TW_SIZE_USERNAME + 1);

  /* Confirm */
  debug("Fetching tweet for %s (%lu)", username, strlen((char *) username));

  /* Fetch tweet */
  error = tw_fetch_tweet(username, tweet);

  if (error != ERROR_NO_ERROR)
  {
    tweet = (unsigned char *) tw_explain_error(error);
  }

  /* Print it */
  printf("%s\n", tweet);

  return 0;
}
