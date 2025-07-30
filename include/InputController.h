#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include <RotaryEncoder.h>
#include <Bounce2.h>

enum InputAction {
    ROTARY_CCW_TICK,
    ROTARY_CW_TICK,
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_PRESS,
    NO_ACTION
};

class InputController {
public:
    InputController(int pinEncoder1, int pinEncoder2, int pinButton);
    void begin();
    void update();

    long int getPosition();
    long int currentPosition;

    bool buttonHeld;
    unsigned long buttonPressStartTime;

    RotaryEncoder encoder;
    int pinEncoder1;
    int pinEncoder2;
    int pinButton;
    Bounce bounce = Bounce();

    InputAction lastAction = NO_ACTION;

};

#endif