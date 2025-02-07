#include <Arduino.h>

#include "Version.hpp"
#include "GlobalConfiguration.hpp"

bool handleATI(Stream *s, const char *szString, int length) {
  bool raiseError = false;
  int index = 0;
  if(length > 1){
    index = szString[1] - '0';
  }
  switch (index) {
    default:
      raiseError = true;
      break;
    case 0:
      s->printf("%s" "\r", getFWVersion());
      break;
    case 1:
      s->printf("%s" "\r", "Pan & Tilt");
      break;
    case 2:
      s->printf("%s" "\r", "Complete axis info");
      break;
  }
  return(raiseError);
}
