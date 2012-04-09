@echo OFF
setlocal
set CONFIG_FILE=config.txt

echo Enter your Twitter username:
(set /p TWITTER_USERNAME=)

echo Enter search query to filter tweets with. (Leave blank to receive all tweets.)
(set /p TWITTER_QUERY=)
set TWITTER_QUERY=from:%TWITTER_USERNAME% AND (%TWITTER_QUERY%)

echo Your settings have been saved. To change your settings, run this script again.

echo %TWITTER_USERNAME%>%CONFIG_FILE%
echo %TWITTER_QUERY%>>%CONFIG_FILE%

endlocal