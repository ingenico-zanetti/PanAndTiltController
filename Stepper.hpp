#ifndef __STEPPER_HPP_INCLUDED__
#define __STEPPER_HPP_INCLUDED__

class Stepper {
	public:
    Stepper(const char *szName, uint32_t runFrequency, int stepPin, int dirPin, int enablePin);
    const char *getName(void);
    bool requestPosition(int32_t);
    int32_t getPosition(void);
    bool requestPositionDelta(int32_t);

    void setMaxSpeed(uint32_t max);
    uint32_t getMaxSpeed(void);

    void setMinSpeed(uint32_t min);
    uint32_t getMinSpeed(void);

    void setCruiseSpeed(uint32_t speed);
    uint32_t getCruiseSpeed(void);

    void setAcceleration(uint32_t acc);
    uint32_t getAcceleration(void);

    bool run(void);
    bool update(void);
    void setEnable(bool);

    static const int MOVEMENT_IDLE = 0;           // not energized
    static const int MOVEMENT_HOLDING = 1;        // not moving, but energized
    static const int MOVEMENT_ACCELERATING = 2;   // increasing speed movement
    static const int MOVEMENT_CONSTANT_SPEED = 3; // constant speed movement
    static const int MOVEMENT_BRAKING = 4;        // decreasing speed movement

    static const int DIRECTION_STOPPED = 0;
    static const int DIRECTION_FORWARD = 1;
    static const int DIRECTION_BACKWARD = -1;
    
uint32_t currentTicks;
uint32_t totalTicks;
uint32_t totalSteps;
bool updateCurrentSpeed(float newSpeed);
float nextSpeed(void);
float acceleration_times_two;

	private:
    const char *name;
    int frequency;
    int32_t requestedPosition;        // requested absolute position, read by run()
    bool    requestedPositionPending; // a new absolute position has been requested, read and cleared by run()
    int32_t requestedDelta;           // delta position request, read and cleared by run()
    int32_t targetPosition;           // updated in update()
    int32_t currentPosition;          // updated in update()
    uint32_t ticksToNextStep;         // 
    int32_t  direction;               // -1 backward, 1 forward, 0 stopped
    bool     enable;                  // drive motor
    int pioStep;
    int pioDir;
    int pioEnable;
    int mouvementState;
    int nextPioStep; // value to push to pioStep at next call to run()
    int travelSteps;
    int halfTravelSteps;
    int travelTicks;
    int halfTravelTicks;
    int rampSteps;
    int rampTicks;
    int remainingSteps;

    float startSpeed;
    uint32_t startSpeedTicks;
    float cruiseSpeed;
    uint32_t cruiseSpeedTicks;
    float maxSpeed;
    uint32_t maxSpeedTicks;
    float currentSpeed;
    uint32_t currentSpeedTicks;
    float acceleration;
};

extern Stepper panStepper;
extern Stepper tiltStepper;
extern Stepper *getStepper(int c);

#endif // __STEPPER_HPP_INCLUDED__
