#ifndef _TWITTER_H_

/* Constants */
#define TW_SIZE_USERNAME 15
#define TW_SIZE_TWEET    140

/* Errors */
#define ERROR_NO_ERROR          0
#define ERROR_USERNAME_TOO_LONG 1

/* Data Types */
#define tw_error int
#define tw_username unsigned char
#define tw_tweet unsigned char

/* Functions */
tw_error tw_fetch_tweet(const tw_username*, tw_tweet*);
const char *tw_explain_error(tw_error);

#endif
