#ifndef STATE_CONTROLLER_H
#define STATE_CONTROLLER_H

#include "SystemState.h"
#include "DisplayController.h"
#include "LedRingController.h"
#include "PiezoController.h"
#include "InputController.h"

class StateController {
public:
    StateController();
    SystemState getState();

    void begin();
    void update(
        DisplayController& display,
        LedRingController& ledRing,
        PiezoController& piezo,
        InputController& input
    );

    long int getTimer();
    long int getInitialTimer();
    long int getPosition();

    void buttonFeedback(PiezoController& piezo);
    void rotaryFeedback(PiezoController& piezo);

    void enterDeepSleep(DisplayController& display, LedRingController& ledRing);
    void checkDeepSleep(DisplayController& display, LedRingController& ledRing, SystemState state);

    long int currentPosition;
    long newPosition;
    long int currentTimer;
    long int initialTimer;
    unsigned long int countdownTimer;
    unsigned long int positionTimer;        
    unsigned long int lastInteractionTimer;
    unsigned long int timeNow;
    unsigned long int timeElapsed;

private:
    SystemState currentState;
    bool firstTime = true;
};

#endif