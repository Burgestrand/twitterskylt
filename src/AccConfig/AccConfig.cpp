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


char ** indata;

AccConfig::AccConfig(char * configFile) {
  
  // Inits the SD shield
  pinMode(53, OUTPUT);
  SD.begin(4);

  File f = SD.open(configFile);
  
  char tmp;
  char *pek = sd;
  
  int in_size = 0;
  int i = 0;
  int x;
  
  int num = 4;
  indata = (char **) malloc(sizeof(char *)*num);

  while(f.available()) {
    tmp = (char) f.read();

    // check if it's a charachter then store it in buffer sd
    if (!(tmp == '\n' || tmp == ' ' || tmp == '\t' || tmp == '#')) {
      *pek = tmp;
      in_size++;
      pek++;
    }
    // the word is done
    else if (in_size > 0) {
      
      // no more space so allocate some more
      if (i == num-1) {
	num += 4;
	char ** newcopy = (char **) malloc(sizeof(char *) * num);

	// move pointers to the new array
	for (x = 0; x < num-4; x++)
	  newcopy[x] = indata[x];

	// deallocate the old space 
	free(indata);
	indata = newcopy;
      }

      char * config = (char *) malloc(sizeof(char) * (in_size+1));
      indata[i] = config;
      
      // move data from buffer sd to the array
      for (x = 0; x < in_size; x++)
	config[x] = sd[x];

      config[x] = '\0';
      pek = sd;
      in_size = 0;
      i++;
    }
    // else multiple whitespaces, ignore sign and loop again
  }
  
  // insert null so we know theres no more relevant data
  indata[i] = '\0';
  f.close();
  
}

char * AccConfig::getUsername() {
  return indata[0];
}

char ** AccConfig::getHashtags() {
  return indata + 1;
}
