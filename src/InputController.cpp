#include <Arduino.h>
#include "InputController.h"
#include "config.h"

InputController::InputController(int pinEncoder1, int pinEncoder2, int pinButton)
    : pinEncoder1(pinEncoder1),
      pinEncoder2(pinEncoder2),
      pinButton(pinButton),
      encoder(RotaryEncoder(pinEncoder1, pinEncoder2, RotaryEncoder::LatchMode::TWO03)) {}

void InputController::begin() {
    pinMode(pinButton, INPUT_PULLUP);
    bounce.attach(pinButton, INPUT_PULLUP);
    bounce.interval(5);
    buttonHeld = false;
    
    // Retain the previous position value. This is defined as an extern variable in config and main
    encoder.setPosition(previousPosition);
}

void InputController::update() {
    lastAction = NO_ACTION; // Reset last action

    // Update the button state
    bounce.update();

    // Check for button press
    if (bounce.fell()) {
        buttonPressStartTime = millis();
        buttonHeld = true;
    }
    
    // Check for long press
    if (buttonHeld && bounce.read() == LOW) {
        if (millis() - buttonPressStartTime > BUTTON_LONG_PRESS_THRESHOLD) {
            lastAction = BUTTON_LONG_PRESS;
            buttonHeld = false; // Prevent multiple long press events
        }
    }

    // Reset when button released
    if (bounce.rose() && buttonHeld) {
        buttonHeld = false;
        lastAction = BUTTON_SHORT_PRESS;
    }

    // Update the rotary encoder state
    encoder.tick();
    long int newPosition = encoder.getPosition();

    if (newPosition != currentPosition) {
        currentPosition = newPosition;
        if (newPosition > previousPosition) {
            lastAction = ROTARY_CW_TICK;
        } else {
            lastAction = ROTARY_CCW_TICK;
        }
    }
}

long int InputController::getPosition() {
    return currentPosition;
}

