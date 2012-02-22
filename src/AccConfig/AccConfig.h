#include <SD.h>

class AccConfig {
  
 private:
 
  char sd[160];

 public:
  
  AccConfig(char * configFile);
  char * getUsername();
  char ** getHashtags();
};

