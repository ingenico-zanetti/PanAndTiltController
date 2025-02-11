#include <Arduino.h>
#include "GlobalConfiguration.hpp"
#include "Stepper.hpp"

static int countComas(const char *szString, int length){
  int count = 0;
  const char *p = szString;
  while(length--){
    if(',' == *p++){
      count++;
    }
  }
  // Serial.printf("%s(\"%s\")=>%d" "\n", __func__, szString, count);
  return(count);
}

typedef bool (*plusSubFunction)(Stream *s, Stepper *stepper, const char c, const char *szString, int comas);
/*static*/ const char *useATW =  "(use AT&W to make the setting persistent)";

static bool plusUsage(Stream *s, Stepper *stepper, const char c, const char *szString, int comas){
  (void)s;
  (void)stepper;
  (void)c;
  (void)szString;
  (void)comas;
  return(false);
}

static bool plusRead(Stream *s, Stepper *stepper, const char c, const char *szString, int comas){
  (void)s;
  (void)stepper;
  (void)c;
  (void)szString;
  (void)comas;
#ifdef __LOG_STEPPER__
  s->printf("%s: position = %d, speed [ %d %d %d ], acceleration = %d (rampSteps=%u) log[%s]" "\n", stepper->getName(), stepper->getPosition(), stepper->getMinSpeed(), stepper->getCruiseSpeed(), stepper->getMaxSpeed(), stepper->getAcceleration(), stepper->getRampSteps(), stepper->getLogString());
#else
  s->printf("%s: position = %d, speed [ %d %d %d ], acceleration = %d (rampSteps=%u) lastError=[%s]" "\n", stepper->getName(), stepper->getPosition(), stepper->getMinSpeed(), stepper->getCruiseSpeed(), stepper->getMaxSpeed(), stepper->getAcceleration(), stepper->getRampSteps(), stepper->getLastError());
#endif
  return(false);
}

/*
 * Stepper write command. Allows to specifiy both absolute and relative move, with speed limit
 * or movement time duration.
 * With X in P for Pan and T for TILT, the following movements can be specified
 *
 * AT+X=ppp           => absolute position
 * AT+X=ppp,ss.s      => absolute position with speed specified as time to reach the requested position (in second)
 * AT+X={+|-}rrr      => relative position
 * AT+X={+|-}rrr,ss.s => relative position with speed specified as time to reach the requested position (in second)
 * AT+X=A,aaa         => specify acceleration in step per second squared for all future moves
 * AT+X=S,sss         => specify cruising speed in step per second for all future moves
 * AT+X=I,sss         => specify starting speed in step per second for all future moves
 * AT+X=X,sss         => specify maximum speed in step per second for all future moves
 */

static bool plusWrite(Stream *s, Stepper *stepper, const char c, const char *szString, int comas){
  s->printf("%s(%c, \"%s\", %d)" "\n", __func__, c, szString, comas);
  bool raiseError = false;
  (void)s;
  (void)stepper;
  (void)c;
  char *end = NULL;
  const char *numberPtr = szString + 3;
  float firstValue = strtof(numberPtr, &end);
  bool hasFirstValue = (end != numberPtr);
  float secondValue = 0.0f;
  bool hasSecondValue = false;
  if(comas > 0){
    numberPtr = strchr(numberPtr, ',') + 1;
    secondValue = strtof(numberPtr, &end);
    hasSecondValue = (end != numberPtr);
  }
  switch(szString[3]){
    case 'E':
      stepper->setEnable(true);
    break;
    case 'D':
      stepper->setEnable(false);
    break;
    case 'S':
      // set cruise speed in step per second
      if(hasSecondValue){
        raiseError = stepper->setCruiseSpeed(secondValue);
      }else{
        raiseError = true;
      }
    break;
    case 'A':
      // set acceleration in step per second squared
      // set cruise speed in step per second
      if(hasSecondValue){
        raiseError = stepper->setAcceleration(secondValue);
      }else{
        raiseError = true;
      }
    break;
    case 'I':
      // set min / start speed in step per second
      if(hasSecondValue){
        raiseError = stepper->setMinSpeed(secondValue);
      }else{
        raiseError = true;
      }
    break;
    case 'X':
      // set max speed in step per second
      if(hasSecondValue){
        raiseError = stepper->setMaxSpeed(secondValue);
      }else{
        raiseError = true;
      }
    break;
    case '-':
    case '+':
      // delta move
      if(hasFirstValue){
        if(hasSecondValue){
          raiseError = stepper->requestTimedPositionDelta((int32_t)firstValue, secondValue);
        }else{
          raiseError = stepper->requestPositionDelta((int32_t)firstValue);
        }
      }else{
        raiseError = true;
      }
    break;
    case 'R':
      raiseError = stepper->resetPosition();
    break;
    default:
      // absolute move
      if(hasFirstValue){
        if(hasSecondValue){
          raiseError = stepper->requestTimedPosition((int32_t)firstValue, secondValue);
        }else{
          raiseError = stepper->requestPosition((int32_t)firstValue);
        }
      }else{
        raiseError = true;
      }
    break;
  }
  return(raiseError);
}

bool handlePlus(Stream *s, const char *szString, int length) {
  bool raiseError = true;
  int comas = countComas(szString, length);
  int commandLength = length;
  char axis = '\0';
  bool isEqual = false;
  Stepper *stepper = NULL;
  plusSubFunction sub = NULL;
  if(commandLength > 1){
    axis = szString[1];
    if(commandLength > 2){
      if('?' == szString[2]){
        sub = plusRead;
      }else{
        isEqual = ('=' == szString[2]);
      }
      if((commandLength > 3) && (isEqual)){
        if('?' == szString[3]){
          sub = plusUsage;
        }else{
          sub = plusWrite;
        }
      }
    }
  }
  if(sub){
    stepper = getStepper(axis);
    if(stepper){
      raiseError = sub(s, stepper, axis, szString, comas);
    }
  }
  // s->printf("%s(\"%s\")=>%d" "\n", __func__, szString, raiseError);
  return (raiseError);
}
