@echo OFF
setlocal
set CONFIG_FILE=config.txt

echo Enter your Twitter username:
set /p USERNAME=
set QUERY=from:%USERNAME%

echo Enter search query to filter tweets with. (Leave blank to receive all tweets.)
set /p CUSTOM_QUERY=
if defined CUSTOM_QUERY set QUERY=%QUERY% AND (%CUSTOM_QUERY%)
set QUERY=%QUERY% +exclude:retweets

echo Your settings have been saved. To change your settings, run this script again.

echo %USERNAME%>%CONFIG_FILE%
echo %QUERY%>>%CONFIG_FILE%

endlocal