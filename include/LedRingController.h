#ifndef LEDRING_CONTROLLER_H
#define LEDRING_CONTROLLER_H

#include <Adafruit_NeoPixel.h>
#include "SystemState.h"
#include "LedRingAnimations.h"
#include "config.h"

class LedRingController {
public:
    LedRingController(int numLeds, int ledPin);

    void begin(int brightness);
    void update(SystemState state, long int encoder, long int timer, long int initialTimer);

    void LedRingTimeScreen(long int timer, long int encoder);
    void LedRingTimeCountdown(long int timer, long int initialTimer, long int encoder);
    void LedRingCountdownPaused(long int timer, long int initialTimer, long int encoder);
    void LedRingStopwatchRun(long int timer);
    void LedRingModeSelect(long int encoder);
    void LedRingSleep();

    void setBrightness(int brightness) {
        this->brightness = brightness;
        strip.setBrightness(brightness);
    }

    void LedringSingleColor(uint32_t color) {
        for (int i = 0; i < numLeds; i++)
            strip.setPixelColor(i, color);
        strip.show();
    }

    int numLeds;
    int ledPin;
    int brightness;

    bool animationRunning;
    void startAnimation(ledRingAnimation animation, long int timer, long int initialTimer, long int encoder);
    void updateAnimation();
    void fadeBetweenStates(uint32_t* initialState, uint32_t* endState);
    void pulseBetweenStates(uint32_t* initialState, uint32_t* endState);
    ledRingAnimation animation;

    int timeScale(long int encoder);
    void invertRing(const uint32_t* ring, uint32_t* output, int arraySize);
    void singleColorRingSelecting(long int timer, bool reversed, uint32_t color, uint32_t* output, int arraySize = NUM_LEDS);
    void singleColorRingCounting(long int timer, long int initialTimer, bool reversed, uint32_t color, uint32_t* output, int arraySize = NUM_LEDS);
    void singleColorRing(uint32_t color, uint32_t* output, int arraySize = NUM_LEDS);
    
    uint32_t blendColor(uint32_t c1, uint32_t c2, uint8_t t);
    uint8_t beatsin8(float bpm, uint8_t low, uint8_t high);

    int totalFrames;
    int currentFrame;
    int animationDuration;
    int animationStartTime;
    int lastFrameTime;
    int frameDelay = 5;

private:
    Adafruit_NeoPixel strip;
    uint32_t initialState[NUM_LEDS];
    uint32_t endState[NUM_LEDS];
    uint32_t newLeds[NUM_LEDS];
};

#endif