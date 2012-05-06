/**

Class to read configuration file from SD card

using example:

create object with the config file as parameter
AccConfig conf = AccConfig("konf.txt");

conf.getUsername, returns pointer to null-terminated char array

char ** hashtags = conf.getHashtags, return pointer to char array with hashtags
hashtags[0] get the 1st hashtag ...
when there is no more hashtags hashtags[i] is null

**/

#include "AccConfig.h"

int AccConfig::begin(const char * _configFile) {
  buf = NULL;
  char *configFile = strclone(_configFile);

  // Inits the SD shield
  pinMode(53, OUTPUT);

  if (!SD.begin(4)) {
    //SD.begin failed
    return 3;
  }

  if (!SD.exists(configFile)) {
    // file don't exists
    return 5;
  }

  File f = SD.open(configFile);
  xfree(configFile);

  if (!f) {
    // couldn't open file
    return 1;
  }

  unsigned long file_size = f.size();

  buf = ALLOC_STR(file_size);

  if (buf == NULL) {
    //couln't allocate memory
    return 4;
  }

  char tmp;
  char * pek = buf;

  /*
    whitespace condition
    0: start, before the non-whitespace
    1: the username
    2: whitespace after the username
    3: a char belongs to the query
   */
  int wscond = 0;

  while(f.available()) {
    tmp = (char) f.read();

    switch (wscond) {

    case 0:

      if (!(pek == buf && isWhitespace(tmp))) {
	*pek = tmp;
	pek++;
	wscond = 1;
      }
      break;

    case 1:

      if (isWhitespace(tmp)) {
	*pek = '\0';
	pek++;
	wscond = 2;
      }
      else {
	*pek = tmp;
	pek++;
      }
      break;

    case 2:

      if (!(isWhitespace(tmp))) {
	*pek = tmp;
	pek++;
	wscond = 3;
      }
      break;

    default:
      *pek = tmp;
      pek++;
    }
  }


  if (wscond == 0) {
    // no username in file
    buf = NULL;
    return 2;
  }

  // loops while theres no space at the end of query
  do {
    pek--;
  } while(*pek < 33);

  // adds a null char at end
  pek++;
  *pek = '\0';

  unsigned long diff = (pek - buf) + 1; // have to include all numbers
  if (file_size != diff) {
    char * new_ptr = (char *) realloc(buf, diff);
    if (new_ptr != NULL)
      buf = new_ptr;
    else {
      // couldn't allocate more memory
      return 4;
    }
  }

  if (wscond == 2) {
    // no query in file, make query with the given username
    query = (char*) malloc(sizeof(char) * ((sizeof(buf) + 6))); //from: is 5 chars

    strcpy(query,"from:");
    strcat(query,buf);

  }
  else {
    pek = buf;
    while (*pek != '\0')
      pek++;
    pek++;
    query = pek;
  }

  f.close();
  return 0; // everything was ok
}

// returns null pointer if no username found
char * AccConfig::getUsername() {
  return buf;
}

// returns null pointer if no query found
char * AccConfig::getQuery() {
  return query;
}

// returns true if whitespace, otherwise false
bool AccConfig::isWhitespace (char chr){
  return  (chr == ' ' || chr == '\n' || chr == '\t' || chr == '\r');
}

