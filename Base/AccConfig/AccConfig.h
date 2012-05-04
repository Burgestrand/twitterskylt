#include <SD.h>

class AccConfig {
  
 private:
 
  char * query;
  char * buf;
  bool isWhitespace(char chr);

 public:
  
  int begin(char * configFile);
  char * getUsername();
  char * getQuery();
};

