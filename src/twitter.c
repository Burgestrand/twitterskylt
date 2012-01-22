#include "twitter.h"
#include <string.h>
#include <stdio.h>

const char *tw_explain_error(tw_error error)
{
  switch (error)
  {
    case ERROR_USERNAME_TOO_LONG:
      return "username is too long (longer than 15 chars)";

    default:
      return "UNKNOWN ERROR";
  }
}

/*
 * Fetches the most recent tweet from the given username.
 *
 * parameters:
 *   - username: max 15 bytes
 *   - tw_tweet: (NUL-terminated string) should fit TW_SIZE_TWEET + 1 bytes
 *
 * returns:
 *   - error status
 */
tw_error tw_fetch_tweet(tw_username username, tw_tweet* tweet)
{
  const char *api_format_url = "https://api.twitter.com/1/statuses/user_timeline.json?count=1&trim_user=true&include_rts=false&include_entities=true&exclude_replies=true&contributor_details=false&screen_name=%s";

  if (strlen((char *) username) > TW_SIZE_USERNAME)
  {
    return ERROR_USERNAME_TOO_LONG;
  }

  // Make the request

  // Parse the tweet

  // Fake a tweet
  char fake_tweet[strlen("%@ is awesome!") + TW_SIZE_USERNAME + 1];
  sprintf(fake_tweet, "@%s is awesome!", username);

  // Return the tweet
  strlcpy((char *) tweet, fake_tweet, 141);
  return ERROR_NO_ERROR;
}
