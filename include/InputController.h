/**
 * @brief Manages input from a rotary encoder and a button.
 *
 * The `InputController` class is responsible for handling input from a rotary encoder and a button. It provides methods to get the current system state based on the input, as well as to retrieve the current position of the rotary encoder.
 *
 * @param pinEncoder1 The first pin connected to the rotary encoder.
 * @param pinEncoder2 The second pin connected to the rotary encoder.
 * @param pinButton The pin connected to the button.
 */
#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include <RotaryEncoder.h>
#include "SystemState.h"
#include <Bounce2.h>
#include "DisplayController.h"
#include "LedRingController.h"
#include "PiezoController.h"

class InputController {
public:
    InputController(int pinEncoder1, int pinEncoder2, int pinButton);

    SystemState getState();
    RotaryEncoder encoder;

    void begin();
    void update(DisplayController& display, LedRingController& ledRing, PiezoController& piezo);
    long int getPosition();
    long int getTimer();
    long int getInitialTimer();
    void enterDeepSleep(DisplayController& display, LedRingController& ledRing);

    int pinEncoder1;
    int pinEncoder2;
    int pinButton;

    long int currentPosition;
    long int currentTimer;
    long int initialTimer;
    unsigned long int countdownTimer;
    unsigned long int positionTimer;        
    unsigned long int lastInteractionTimer;

private:
    SystemState currentState;
    Bounce bounce = Bounce();
    bool firstTime = true;
};

#endif