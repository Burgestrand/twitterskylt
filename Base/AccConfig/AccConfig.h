#include <SD.h>
#include <Utilities.h>

class AccConfig
{
 private:

  char * query;
  char * buf;
  bool isWhitespace(char chr);

 public:
  int begin(const char * configFile);
  char * getUsername();
  char * getQuery();
};

