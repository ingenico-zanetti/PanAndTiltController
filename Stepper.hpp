#ifndef __STEPPER_HPP_INCLUDED__
#define __STEPPER_HPP_INCLUDED__

#include "GlobalConfiguration.hpp"

#ifdef __LOG_STEPPER__

#define LOG_SIZE (4095)

#endif

class Stepper {
	public:
    Stepper(const char *szName, uint32_t runFrequency, int stepPin, int dirPin, int enablePin);
    const char *getName(void);
    bool requestPosition(int32_t);
    int32_t getPosition(void);
    bool requestPositionDelta(int32_t);

    bool requestTimedPosition(int32_t, float);
    bool requestTimedPositionDelta(int32_t, float);

    bool isValidCruiseSpeed(float speed);
    bool isValidSpeed(float speed);

    bool setMaxSpeed(float speed);
    uint32_t getMaxSpeed(void);

    bool setMinSpeed(float speed);
    uint32_t getMinSpeed(void);

    bool setCruiseSpeed(float speed);
    uint32_t getCruiseSpeed(void);

    bool setAcceleration(float acc);
    uint32_t getAcceleration(void);

    uint32_t getRampSteps(void);
    bool resetPosition(void);

    bool run(void);
    bool update(void);
    void setEnable(bool);

    static const int MOVEMENT_STATE_IDLE = 0;         // not energized
    static const int MOVEMENT_STATE_HOLDING = 1;      // not moving, but energized
    static const int MOVEMENT_STATE_ACCELERATING = 2; // increasing speed movement
    static const int MOVEMENT_STATE_CRUISING = 3;     // constant speed movement
    static const int MOVEMENT_STATE_BRAKING = 4;      // decreasing speed movement

    static const int DIRECTION_STOPPED = 0;
    static const int DIRECTION_FORWARD = 1;
    static const int DIRECTION_BACKWARD = -1;

    static const char noError[];
    
  uint32_t currentTicks;
  uint32_t totalTicks;
  uint32_t totalSteps;
  bool updateCurrentSpeed(float newSpeed);
  float nextSpeed(void);
  float acceleration_times_two;

#ifdef __LOG_STEPPER__
  void log(const char *sz);
  const char *getLogString(void){return(logString);}
#endif
  const char *getLastError(void);
  void clearLastError(void);

	private:
#ifdef __LOG_STEPPER__
    char logString[LOG_SIZE + 1];
#endif
    const char *name;
    int frequency;
    int32_t requestedPosition;        // requested absolute position, read by run()
    bool    requestedPositionPending; // a new absolute position has been requested, read and cleared by run()
    int32_t requestedDelta;           // delta position request, read and cleared by run()
    int32_t targetPosition;           // updated in update()
    int32_t currentPosition;          // updated in update()
    uint32_t ticksToNextStep;         // 
    int32_t  direction;               // -1 backward, 1 forward, 0 stopped
    bool setDirection(int newDir);
    bool enable;                  // drive motor
    int pioStep;
    int pioDir;
    int pioEnable;
    int movementState;
    int nextPioStep; // value to push to pioStep at next call to run()
    // speed-controlled move
    int remainingSteps;
    int travelSteps;
    int halfTravelSteps;
    int rampSteps;

    float startSpeed;
    uint32_t startSpeedTicks;
    float cruiseSpeed;
    uint32_t cruiseSpeedTicks;
    float maxSpeed;
    uint32_t maxSpeedTicks;
    float currentSpeed;
    uint32_t currentSpeedTicks;
    float acceleration;

    // time-controlled move
    uint32_t pendingTravelTicks;
    uint32_t travelTicks;
    uint32_t halfTravelTicks;
    uint32_t rampTicks;
    uint32_t halfRemainingSteps;

    const char *lastError;

};

extern Stepper panStepper;
extern Stepper tiltStepper;
extern Stepper *getStepper(int c);

#endif // __STEPPER_HPP_INCLUDED__
