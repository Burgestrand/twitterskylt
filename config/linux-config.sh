#!/bin/sh
TWITTER_CONFIG_FILE=config.txt

echo "Enter your Twitter username: "
read TWITTER_USERNAME
TWITTER_QUERY="from:$TWITTER_USERNAME"

echo "Enter search query to filter tweets with. (Leave blank to receive all tweets.)"
read CUSTOM_TWITTER_QUERY
if [[ $CUSTOM_TWITTER_QUERY != "" ]]
	then TWITTER_QUERY="$TWITTER_QUERY AND ($CUSTOM_TWITTER_QUERY)"
fi
TWITTER_QUERY="$TWITTER_QUERY +exclude:retweets"

echo "Your settings have been saved. To change your settings, run this script again."

echo $TWITTER_USERNAME > $TWITTER_CONFIG_FILE
echo $TWITTER_QUERY >> $TWITTER_CONFIG_FILE