#include <Arduino.h>

#include "Stepper.hpp"

Stepper panStepper( "PAN",  10000, PB12, PB13, PB14);
Stepper tiltStepper("TILT", 10000, PA8,  PA9,  PA10);

Stepper *getStepper(int c){
  switch(c){
    case 'P':
    return(&panStepper);
    break;
    case 'T':
    return(&tiltStepper);
    break;
  }
  return(NULL);
}

Stepper::Stepper(const char *szName, uint32_t runFrequency, int stepPin, int dirPin, int enablePin){
  name = szName;
  frequency = runFrequency;

  enable = false;
  nextPioStep = LOW;
  pioStep   = stepPin;   pinMode(pioStep, OUTPUT);   digitalWrite(pioStep, nextPioStep); // prepare for rising front
  direction = DIRECTION_STOPPED;
  pioDir    = dirPin;    pinMode(pioDir, OUTPUT);    digitalWrite(pioDir, LOW);          // irrelevant yet
  pioEnable = enablePin; pinMode(pioEnable, OUTPUT); digitalWrite(pioEnable, HIGH);      // disable

  requestedPositionPending = false;
  requestedDelta = 0;
  targetPosition = 0;
  currentPosition = 0;
  startSpeed = 20.0f;
  maxSpeed = 1020.0f;
  acceleration = 2000.0f;

  currentSpeed = (float)startSpeed;
  currentTicks = (int)((float)frequency / currentSpeed);
  totalSteps = 0;
  totalTicks = 0;
  movementState = MOVEMENT_STATE_IDLE;
  ticksToNextStep = currentTicks;
  remainingSteps = 0;

  logString[0] = '\0';

}

bool Stepper::setMaxSpeed(float speed){
  if(remainingSteps){
    return(true);
  }else{
    maxSpeed = speed;
  }
  return(false);
}

uint32_t Stepper::getMaxSpeed(void){
  return((uint32_t)maxSpeed);
}

bool Stepper::setMinSpeed(float speed){
  if(remainingSteps){
    return(true);
  }else{
    startSpeed = speed;
  }
  return(false);
}

uint32_t Stepper::getMinSpeed(void){
  return((uint32_t)startSpeed);
}

bool Stepper::setCruiseSpeed(float speed){
  bool raiseError = true;
  if(0 == remainingSteps){
    if(isValidCruiseSpeed(speed)){
      cruiseSpeed = speed;
      raiseError = false;
    }
  }
  return(raiseError);
}

uint32_t Stepper::getCruiseSpeed(void){
  return((uint32_t)cruiseSpeed);
}

bool Stepper::setAcceleration(float acc){
  if(remainingSteps){
    return(true);
  }else{
    acceleration = acc;
  }
  return(false);
}

uint32_t Stepper::getAcceleration(void){
  return((uint32_t)acceleration);
}

const char *Stepper::getName(void){
  return(name);
}

int32_t Stepper::getPosition(void){
  return(currentPosition);
}

bool Stepper::resetPosition(void){
  if(remainingSteps){
    return(true);
  }else{
    currentPosition = 0;
  }
  return(false);
}

bool Stepper::requestPosition(int32_t position){
  bool raiseError = false;
  if(remainingSteps > 0){
    raiseError = true;
  }else{
    requestedPosition = position;
    requestedPositionPending = true;
  }
  return(raiseError);
}

bool Stepper::requestPositionDelta(int32_t delta){
  bool raiseError = false;
  if(remainingSteps > 0){
    raiseError = true;
  }else{
    requestedDelta = delta;
  }
  return(raiseError);
}

bool Stepper::requestTimedPosition(int32_t position, float duration){
  if(0.0f == travelTicks){
    travelTicks = duration * frequency;
    return(requestPosition(position));
  }
  return(true);
}

bool Stepper::requestTimedPositionDelta(int32_t delta, float duration){
  if(0.0f == travelTicks){
    travelTicks = duration * frequency;
    return(requestPositionDelta(delta));
  }
  return(true);
}

void Stepper::setEnable(bool e){
  enable = e;
  // Active low pin
  if(enable){
    digitalWrite(pioEnable, LOW); 
  }else {
    digitalWrite(pioEnable, HIGH); 
  }
}

#ifdef __LOG__
static void sprintfFloat(char *szString, float f){
  int entier = (int)floor(f);
  int virgule = (int)((f - floor(f)) * 1000000.0);
  sprintf(szString, "%d.%06d", entier, virgule);
}
#endif

bool Stepper::isValidCruiseSpeed(float newSpeed){
  return((0.0f == newSpeed) || ((startSpeed <= newSpeed) && (newSpeed <= maxSpeed)));
}

bool Stepper::isValidSpeed(float newSpeed){
  if(0.0f == cruiseSpeed){
#ifdef __LOG__
    char newSpeedString[32];
    sprintfFloat(newSpeedString, newSpeed);
    log("isValidSpeed("); log(newSpeedString); log(");");
#endif
    return((startSpeed <= floor(newSpeed + 0.5)) && (newSpeed <= maxSpeed));
  }else{
    return((startSpeed <= newSpeed) && (newSpeed <= cruiseSpeed));
  }
}

bool Stepper::updateCurrentSpeed(float newSpeed){
  if(isValidSpeed(newSpeed)){
    currentSpeed = newSpeed;
    currentTicks = (int)((float)frequency / currentSpeed);
    return(true);
  }
  return(false);
}

void Stepper::log(const char *sz){
  unsigned int offset = strlen(logString);
  unsigned int length = strlen(sz);
  if((offset + length) >= (sizeof(logString) - 1)){
    memmove(logString, logString + length, (offset - length));
    offset -= length;
  }
  strcpy(logString + offset, sz);
}

float Stepper::nextSpeed(void) {
    float square = currentSpeed * currentSpeed + acceleration_times_two;
    float returnValue = 0.0;
    if (square > 0.0) {
        returnValue = sqrt(square);
    }
    return(returnValue);
}

bool Stepper::run(void){
  digitalWrite(pioStep, nextPioStep);
  nextPioStep = LOW;
  return(true);
}

bool Stepper::setDirection(int newDir){
  if(direction == newDir){
    return(false);
  }else{
    direction = newDir;
    if(DIRECTION_FORWARD == direction){
      digitalWrite(pioDir, LOW);
    }else if(DIRECTION_BACKWARD == direction){
      digitalWrite(pioDir, HIGH);
    }
    return(true);
  }
}

bool Stepper::update(void){
  if(remainingSteps > 0){
    if(ticksToNextStep > 0){
      ticksToNextStep--;
      if(0 == ticksToNextStep){
        bool fakeCruise = false;
        remainingSteps--;
        totalSteps++;
        currentPosition += direction;
        // what to do next ?
        // If we are still accelerating and have reached half the travel distance, we need to start breaking
        // If we are cruising and we have just rampSteps of travel distance left, we need to start breaking
        if((MOVEMENT_STATE_ACCELERATING == movementState) && (remainingSteps <= halfTravelSteps)){
          if(travelSteps & 1){
            movementState = MOVEMENT_STATE_BRAKING;
            acceleration_times_two = -acceleration_times_two;
          }else{
            // even number of steps => add a "cruising" step
            movementState = MOVEMENT_STATE_CRUISING;
            fakeCruise = true;
          }
        }
        if((MOVEMENT_STATE_CRUISING == movementState)){
          if(fakeCruise || (remainingSteps <= rampSteps)){
            movementState = MOVEMENT_STATE_BRAKING;
            acceleration_times_two = -acceleration_times_two;
          }
        }else{
          bool speedUpdated = updateCurrentSpeed(nextSpeed());
          if(!speedUpdated && (MOVEMENT_STATE_ACCELERATING == movementState)){
            // Cruising speed reached
            movementState = MOVEMENT_STATE_CRUISING;
            rampSteps = totalSteps;
          }
        }
        ticksToNextStep = currentTicks;
        totalTicks += ticksToNextStep;
        nextPioStep = HIGH; // rising edge at next call to run()
      }
    }
    return(true);
  }else{
    int remaining = 0;
    // check for pending move request
    if(requestedDelta){
      requestedPositionPending = false;
      if(requestedDelta < 0){
        remaining = -requestedDelta;
        setDirection(DIRECTION_BACKWARD);
      }else{
        remaining = requestedDelta;
        setDirection(DIRECTION_FORWARD);
      }
      requestedDelta = 0;
    }else if(requestedPositionPending){
      requestedPositionPending = false;
      if(requestedPosition < currentPosition){
        remaining = (currentPosition - requestedPosition);
        setDirection(DIRECTION_BACKWARD);
      }else{
        remaining = (requestedPosition - currentPosition);
        setDirection(DIRECTION_FORWARD);
      }
    }
    if(remaining > 0){
      // configure the first step
      currentSpeed = (float)startSpeed;
      currentTicks = (int)((float)frequency / currentSpeed);
      ticksToNextStep = currentTicks;
      acceleration_times_two = 2.0f * acceleration;
      totalSteps = 0;
      totalTicks = 0;
      rampSteps = 0;
      travelSteps = remaining;
      halfTravelSteps = (remaining >> 1); // 1 => 0, 2 => 1, 3 => 1, 4 => 2, ...
      movementState = MOVEMENT_STATE_ACCELERATING;
      remainingSteps = remaining;
    }
  }
  return(false);
}

uint32_t Stepper::getRampSteps(void){
  return(rampSteps);
}
