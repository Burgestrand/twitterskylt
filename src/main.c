#include "util.h"
#include "twitter.h"
#include <stdio.h>

int
main(void)
{
  unsigned char *username = ALLOC_STR(TW_SIZE_USERNAME);
  unsigned char *tweet    = ALLOC_STR(TW_SIZE_TWEET);

  // Read username to fetch for
  printf("Username: ");
  username = getstr(username, TW_SIZE_USERNAME + 1);

  // Confirm
  debug("Fetching tweet for %s (%ld)", username, strlen(username));

  // Fetch tweet
  tw_error error = tw_fetch_tweet(username, tweet);

  if (error != ERROR_NO_ERROR)
  {
    tweet = (unsigned char *) tw_explain_error(error);
  }

  // Print it
  printf("%s\n", tweet);

  return 0;
}
