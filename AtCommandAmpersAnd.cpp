#include <Arduino.h>
#include <Stepper.hpp>
#include "GlobalConfiguration.hpp"


extern bool handleAmpersAnd(Stream *s, const char *szString, int length){
  bool raiseError = false;
  if(2 == length){
    switch(szString[1]){
      case 'V':
        s->printf("%s: %d" "\n", panStepper.getName(), panStepper.getPosition());
        s->printf("%s: %d" "\n", tiltStepper.getName(), tiltStepper.getPosition());
      break;
      default:
        raiseError = true;
      break;
    } 
  }else{
    raiseError = true;
  }
  // Serial.printf("%s(\"%s\", %d)=>%d" "\n", __func__, szString, length, raiseError);
  return(raiseError);
}
