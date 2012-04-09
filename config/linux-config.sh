#!/bin/sh
CONFIG_FILE=config.txt

echo "Enter your Twitter username: "
read TWITTER_USERNAME

echo "Enter search query to filter tweets with. (Leave blank to receive all tweets.)"
read TWITTER_QUERY
TWITTER_QUERY="from:$TWITTER_USERNAME AND ($TWITTER_QUERY)"

echo "Your settings have been saved. To change your settings, run this script again."

echo $TWITTER_USERNAME > $CONFIG_FILE
echo $TWITTER_QUERY >> $CONFIG_FILE