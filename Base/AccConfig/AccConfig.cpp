/**

Class to read configuration file from SD card

using example:

create object with the config file as parameter
AccConfig conf = AccConfig("konf.txt");

conf.getUsername, returns pointer to null-terminated char array

conf.getQuery, returns pointer to null-terminated char array

**/

/**
   Error codes:
   1: Unable to open the file
   2: No username in the file
   3: SD.begin failed
   4: Unable to allocate memory for buffer
   5: No file on SD card corresponds to given file name

   If everything is all right zero will be returned
 **/


#include "AccConfig.h"

int AccConfig::begin(char * configFile) {
 
  buf = NULL; 
  
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
  
  if (!f) {
    // couldn't open file
    return 1;
  }
  
  unsigned long file_size = f.size();
  
  buf = (char *) malloc(sizeof(char) * (file_size + 1));

  if (buf == NULL) {
    //couldn't allocate memory
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
  } //end of while(f.available())

  if (wscond == 0) {
    // no username in file
    buf = NULL;
    return 2;
  }
  
  // put the pointer pek to the last char of importance
  do {
    pek--;
  } while(*pek < 33); //ASCII 33 is '!'
  
  // adds a null char at end
  pek++;
  *pek = '\0';

  int diff = (pek - buf) + 1; // have to include all numbers
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
    // "from:" is 5 chars and " +exclude:retweets" is 18 chars
    query = (char*) malloc(sizeof(char) * ((strlen(buf) + 24)));

    strcpy(query,"from:");
    strcat(query,buf);
    strcat(query," +exclude:retweets"); //space is important!

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

// returns true if whitespace, otherwise false
bool AccConfig::isWhitespace (char chr){
  return  (chr == ' ' || chr == '\n' || chr == '\t' || chr == '\r');
}


// returns null pointer if no username found
char * AccConfig::getUsername() {
  return buf;
}

// returns default query if no query found
// undefined if there is no username
char * AccConfig::getQuery() {
  return query;
}

