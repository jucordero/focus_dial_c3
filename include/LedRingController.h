#ifndef LEDRING_CONTROLLER_H
#define LEDRING_CONTROLLER_H

#include <FastLED.h>
#include "SystemState.h"
#include "LedRingAnimations.h"

class LedRingController {
public:
    LedRingController(int numLeds, int ledPin, int brightness);

    void begin();
    void update(SystemState state, long int encoder, long int timer, long int initialTimer);

    void LedRingTimeScreen(long int timer, long int encoder);
    void LedRingTimeCountdown(long int timer, long int initialTimer, long int encoder);
    void LedRingCountdownPaused(long int timer, long int initialTimer, long int encoder);
    void LedRingStopwatchRun(long int timer);
    void LedRingModeSelect(long int encoder);
    void LedRingSleep();

    int numLeds;
    int ledPin;
    int brightness;

    bool animationRunning;
    void startAnimation(ledRingAnimation animation, long int timer, long int initialTimer, long int encoder);
    void updateAnimation();
    void fadeBetweenStates(CRGB* initialState, CRGB* endState);
    void pulseBetweenStates(CRGB* initialState, CRGB* endState);
    ledRingAnimation animation;

    int timeScale(long int encoder);
    void invertRing(const CRGB* ring, CRGB* output, int arraySize);
    void singleColorRingSelecting(long int timer, bool reversed, CRGB color, CRGB* output, int arraySize = 16);
    void singleColorRingCounting(long int timer, long int initialTimer, bool reversed, CRGB color, CRGB* output, int arraySize = 16);
    void singleColorRing(CRGB color, CRGB* output, int arraySize = 16);
    
    int totalFrames;
    int currentFrame;
    int animationDuration;
    int animationStartTime;
    int lastFrameTime;
    int frameDelay = 5;

private:
    CRGB* leds; // Dynamically allocated LED array
    CRGB initialState[16];
    CRGB endState[16];
    CRGB newLeds[16];
};

#endif