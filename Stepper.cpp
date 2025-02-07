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

bool Stepper::updateCurrentSpeed(float newSpeed){
  if((startSpeed <= newSpeed) && (newSpeed <= maxSpeed)){
    currentSpeed = newSpeed;
    currentTicks = (int)((float)frequency / currentSpeed);
    return(true);
  }
  return(false);
}

Stepper::Stepper(const char *szName, uint32_t runFrequency, int stepPin, int dirPin, int enablePin){
  name = szName;
  frequency = runFrequency;

  enable = false;
  nextPioStep = LOW;
  pioStep   = stepPin;   pinMode(pioStep, OUTPUT);   digitalWrite(pioStep, nextPioStep); // prepare for rising front
  pioDir    = dirPin;    pinMode(pioDir, OUTPUT);    digitalWrite(pioDir, LOW);          // irrelevant yet
  pioEnable = enablePin; pinMode(pioEnable, OUTPUT); digitalWrite(pioEnable, HIGH);      // disable

  requestedPositionPending = false;
  requestedDelta = 0;
  targetPosition = 0;
  currentPosition = 0;
  direction = DIRECTION_STOPPED;
  startSpeed = 20.0f;
  maxSpeed = 1020.0f;
  acceleration = 1000.0f;

  currentSpeed = (float)startSpeed;
  currentTicks = (int)((float)frequency / currentSpeed);
  totalSteps = 0;
  totalTicks = 0;
  mouvementState = MOVEMENT_IDLE;
  ticksToNextStep = currentTicks;
  remainingSteps = 0;
}

void Stepper::setMaxSpeed(uint32_t max){
  maxSpeed = (float)max;
}

uint32_t Stepper::getMaxSpeed(void){
  return((uint32_t)maxSpeed);
}

void Stepper::setMinSpeed(uint32_t min){
  startSpeed = (float)min;
}

uint32_t Stepper::getMinSpeed(void){
  return((uint32_t)startSpeed);
}

void Stepper::setCruiseSpeed(uint32_t speed){
  cruiseSpeed = (float)speed;
}

uint32_t Stepper::getCruiseSpeed(void){
  return((uint32_t)cruiseSpeed);
}

void Stepper::setAcceleration(uint32_t acc){
  acceleration = (float)acc;
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

void Stepper::setEnable(bool e){
  enable = e;
  // Active low pin
  if(enable){
    digitalWrite(pioEnable, LOW); 
  }else {
    digitalWrite(pioEnable, HIGH); 
  }
}


#if 0
static void printfFloat(float f){
  int entier = (int)floor(f);
  int virgule = (int)((f - floor(f)) * 1000000.0);
  Serial.printf("%d.%06d", entier, virgule);
}
#endif

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

bool Stepper::update(void){
  if(remainingSteps > 0){
    if(ticksToNextStep > 0){
      ticksToNextStep--;
      if(0 == ticksToNextStep){
        remainingSteps--;
        currentPosition += direction;
        updateCurrentSpeed(nextSpeed());
        ticksToNextStep = currentTicks;
        totalTicks += ticksToNextStep;
        totalSteps++;
        nextPioStep = HIGH;
      }
    }
    return(true);
  }else{
    // check for pending move request
    if(requestedDelta){
      requestedPositionPending = false;
      if(requestedDelta < 0){
        remainingSteps = -requestedDelta;
        direction = DIRECTION_BACKWARD;
      }else{
        remainingSteps = requestedDelta;
        direction = DIRECTION_FORWARD;
      }
      requestedDelta = 0;
    }else if(requestedPositionPending){
      requestedPositionPending = false;
      if(requestedPosition < currentPosition){
        remainingSteps = (currentPosition - requestedPosition);
        direction = DIRECTION_BACKWARD;
      }else{
        remainingSteps = (requestedPosition - currentPosition);
        direction = DIRECTION_FORWARD;
      }
    }
    if(remainingSteps > 0){
      // configure the first step
      currentSpeed = (float)startSpeed;
      currentTicks = (int)((float)frequency / currentSpeed);
      ticksToNextStep = currentTicks;
      acceleration_times_two = 2.0f * acceleration;
      totalSteps = 0;
      totalTicks = 0;
    }
  }
  return(false);
}
