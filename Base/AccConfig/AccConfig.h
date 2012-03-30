#include <SD.h>

class AccConfig {
  
 private:
 
  char * query;
  char * buf;

 public:
  
  int begin(char * configFile);
  char * getUsername();
  char * getQuery();
};

