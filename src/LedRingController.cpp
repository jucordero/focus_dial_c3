#include "LedRingController.h"
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "EEPROM.h"

LedRingController::LedRingController(int numLeds, int ledPin)
  : numLeds(numLeds),
    ledPin(ledPin),
    strip(numLeds, ledPin, NEO_GRB + NEO_KHZ800)
{
    // No need to manually allocate CRGB array anymore
}

void LedRingController::begin(int brightness) {
    strip.begin();
    this->brightness = brightness;
    strip.setBrightness(brightness);
    strip.clear();
    strip.show();
}

void LedRingController::update(SystemState state, long int encoder, long int timer, long int initialTimer){
  if (animationRunning) updateAnimation();
  else{
    switch (state)
    {
    case STATE_TIMER_SELECT:
      LedRingTimeScreen(timer, encoder);
      break;
    case STATE_PULSE_SELECT:
      LedRingTimeScreen(timer, encoder);
      break;
    case STATE_TIMER_RUN:
      LedRingTimeCountdown(timer, initialTimer, encoder);
      break;
    case STATE_PULSE_RUN:
      LedRingTimeCountdown(timer, initialTimer, encoder);
      break;
    case STATE_TIMER_PAUSED:
      LedRingCountdownPaused(timer, initialTimer, encoder);
      break;
    case STATE_STOPWATCH_RUN:
      LedRingStopwatchRun(timer);
      break;
    case STATE_MODE_SELECT:
      LedRingModeSelect(abs(encoder));
      break;
    case STATE_SETTINGS_LEDRING:
      LedringSingleColor(strip.Color(255, 255, 255));
      break;
    case STATE_SETTINGS:
      LedringSingleColor(strip.Color(0, 0, 0));
      break;

    default:
      break;
    }
  }
}

void LedRingController::startAnimation(ledRingAnimation anim, long int timer, long int initialTimer, long int encoder){
  animationRunning = true;
  animation = anim;
  currentFrame = 0;
  animationStartTime = millis();

  // Populate initialState with the current LED state
  for (int i = 0; i < numLeds; i++) {
    initialState[i] = strip.getPixelColor(i);
    endState[i] = strip.Color(0,0,0); // Default endState is off (black)
  }

  switch (anim)
  {
  case LEDRING_PAUSE_TIMER:
    totalFrames = 20;
    // assign single color ring in counting state to endState
    singleColorRingCounting(timer, initialTimer, encoder < 0, strip.Color(255, 255, 0), endState);
    break;

  case LEDRING_START_TIMER:
    totalFrames = 20;
    // assign single color ring in counting state to endState
    singleColorRingCounting(timer, initialTimer, encoder < 0, strip.Color(255, 0, 0), endState);
    break;

  case LEDRING_FINISHED_TIMER:
    // assign single color ring initialState and endState
    singleColorRing(strip.Color(0, 0, 0), initialState);
    singleColorRing(strip.Color(255, 255, 255), endState);
    break;

  case LEDRING_RETURN_MAIN_MENU:
    totalFrames = 10;  
    for (int i = abs(encoder)%8*2; i < abs(encoder)%8*2+2; i++){
      endState[i] = strip.Color(0, 255, 255);
    }
    break;

  case LEDRING_PREPARE_SLEEP:
    totalFrames = 20;
    // assign single color ring in counting state to endState
    singleColorRing(strip.Color(0, 0, 0), endState);
    break;

  case LEDRING_MODE_SELECT:
    totalFrames = 10;
    singleColorRing(strip.Color(0, 0, 0), endState);
    break;


  default:
    break;
  }
}

void LedRingController::updateAnimation(){

  if (millis() - lastFrameTime < frameDelay){
    return;
  }
  lastFrameTime = millis();
  switch (animation)
  {
  case LEDRING_PAUSE_TIMER:
    fadeBetweenStates(initialState, endState);
    break;
  
  case LEDRING_START_TIMER:
    fadeBetweenStates(initialState, endState);
    break;

  case LEDRING_FINISHED_TIMER:
    pulseBetweenStates(initialState, endState);
    break;

  case LEDRING_RETURN_MAIN_MENU:
    fadeBetweenStates(initialState, endState);
    break;

  case LEDRING_PREPARE_SLEEP:
    fadeBetweenStates(initialState, endState);
    break;
  
  case LEDRING_MODE_SELECT:
    fadeBetweenStates(initialState, endState);
    break;

  default:
    break;
  }
}

void LedRingController::LedRingTimeScreen(long int timer, long int encoder){

    int DELTA_T_CCW = EEPROM.readInt(EEPROM_DELTAT_CCW_ADDR);
    int DELTA_T_CW = EEPROM.readInt(EEPROM_DELTAT_CW_ADDR);

    strip.clear();
    if (encoder < 0)
      singleColorRingSelecting(timer/DELTA_T_CCW, true, strip.Color(0, 0, 255), newLeds);
    if (encoder >= 0)
      singleColorRingSelecting(timer/DELTA_T_CW, false, strip.Color(0, 255, 0), newLeds);
    for (int i = 0; i < numLeds; i++)
      strip.setPixelColor(i, newLeds[i]);
    strip.show();
}

void LedRingController::LedRingTimeCountdown(long int timer, long int initialTimer, long int encoder){
    
    strip.clear();
    singleColorRingCounting(timer, initialTimer, encoder<0,  strip.Color(255, 0, 0), newLeds);
    for (int i = 0; i < numLeds; i++)
      strip.setPixelColor(i, newLeds[i]);
    strip.show();
}

void LedRingController::LedRingStopwatchRun(long int timer){
    strip.clear();
    singleColorRingCounting(timer%1000, 1000, false, strip.Color(255, 0, 0), newLeds);
    
    for (int i = 0; i < numLeds; i++)
      strip.setPixelColor(i, newLeds[i]);

    strip.show();
}

void LedRingController::LedRingCountdownPaused(long int timer, long int initialTimer, long int encoder){
    
    strip.clear();
    singleColorRingCounting(timer, initialTimer, encoder<0, strip.Color(255, 255, 0), newLeds);
    for (int i = 0; i < numLeds; i++)
      strip.setPixelColor(i, newLeds[i]);
    strip.show();
}

void LedRingController::LedRingSleep(){
    strip.clear();
    strip.show();
}


void LedRingController::fadeBetweenStates(uint32_t* initialState, uint32_t* endState){
  strip.clear();

  float t = (float) currentFrame / totalFrames;
  for (int i = 0; i < numLeds; i++)
    strip.setPixelColor(i, blendColor(initialState[i], endState[i], t*255));
  currentFrame++;
  if (currentFrame > totalFrames)
    animationRunning = false;
  strip.show();
}

void LedRingController::pulseBetweenStates(uint32_t* initialState, uint32_t* endState){
  strip.clear();
  for (int i = 0; i < numLeds; i++)
    strip.setPixelColor(i, blendColor(initialState[i], endState[i], beatsin8(20, 0, 255)));

  strip.show();
}

void LedRingController::LedRingModeSelect(long int encoder){
  strip.clear();
  unsigned int initPos = abs(encoder)%6*4;
  
  for (int i = initPos; i < initPos+4; i++){
    strip.setPixelColor(i, 0, 255, 255);
  }
  strip.show();
}

/**
 * @brief Calculates a time scale factor based on the encoder value.
 *
 * This function determines the smallest power-of-two scale factor such that
 * the encoder value is less than NUM_LEDS times the scale factor. It starts with
 * a scale factor of 1 and doubles it until the condition is met.
 *
 * @param encoder The input value from the encoder.
 * @return The calculated time scale factor.
 */
int LedRingController::timeScale(long int encoder){
  int testExp = 1;
  while (true){
    if (encoder < NUM_LEDS*testExp)
      return testExp;
    else
      testExp *= 2;
  }    
}

/**
 * @brief Inverts the order of LEDs in a ring.
 *
 * Copies the contents of the input LED array (`ring`) into the output array (`output`)
 * in reverse order. The size of the arrays is specified by `arraySize`.
 *
 * @param ring Pointer to the input array of CRGB objects representing the LED ring.
 * @param output Pointer to the output array where the inverted LED order will be stored.
 * @param arraySize Number of elements in the ring and output arrays.
 */
void LedRingController::invertRing(const uint32_t* ring, uint32_t* output, int arraySize) {
  for (int i = 0; i < arraySize; i++)
      output[i] = ring[arraySize - i - 1];
}

/**
 * @brief Sets the LED ring to display a single color selection effect based on a timer value.
 *
 * This function fills the output LED array with a color up to a position determined by the timer.
 * The next LED is blended between black and the selected color to create a smooth transition.
 * If reversed is true, the LED ring order is inverted.
 *
 * @param timer      The current timer value used to determine the selection position.
 * @param reversed   If true, the LED ring order is inverted.
 * @param color      The CRGB color to display on the selected LEDs.
 * @param output     Pointer to the output array of CRGB LEDs to be updated.
 * @param arraySize  The number of LEDs in the output array.
 */
void LedRingController::singleColorRingSelecting(long int timer, bool reversed, uint32_t color, uint32_t* output, int arraySize) {
  for (int i = 0; i < arraySize; i++)
    output[i] = strip.Color(0, 0, 0);

  int d = timeScale(timer);
  int posLeds = timer / d;
  int remLeds = timer % d;

  for (int i = 0; i < posLeds; i++)
    output[i] = color;

  output[posLeds] = blendColor(strip.Color(0, 0, 0), color, remLeds * 255 / d);

  if (reversed) {
      uint32_t temp[arraySize];
      invertRing(output, temp, arraySize);
      for (int i = 0; i < arraySize; i++)
          output[i] = temp[i];
    }
}

/**
 * @brief Updates an LED ring to display a single color progress indicator.
 *
 * This function fills an output array representing an LED ring with a color pattern
 * based on the elapsed timer value. The LEDs are colored to indicate progress,
 * with a smooth transition on the current progress LED. Optionally, the ring can be reversed.
 *
 * @param timer         The current timer value indicating elapsed time.
 * @param initialTimer  The initial timer value representing the total duration.
 * @param reversed      If true, the LED ring pattern is inverted.
 * @param color         The CRGB color to use for the progress indication.
 * @param output        Pointer to the output array of CRGB LEDs to be updated.
 * @param arraySize     The number of LEDs in the output array.
 */
void LedRingController::singleColorRingCounting(
  long int timer,
  long int initialTimer,
  bool reversed,
  uint32_t color,
  uint32_t* output,
  int arraySize
) {
  int brightnessStep = 10;
  for (int i = 0; i < arraySize; i++)
    output[i] = strip.Color(0, 0, 0);

  long int elapsed = (timer * NUM_LEDS * brightnessStep) / initialTimer;

  int posLeds = elapsed / brightnessStep;
  int remLeds = elapsed % brightnessStep;

  for(int i = 0; i < posLeds; i++)
    output[i] = color;

  output[posLeds] = blendColor(strip.Color(0, 0, 0), color, remLeds * 255 / brightnessStep);

  if (reversed) {
    uint32_t temp[arraySize];
    invertRing(output, temp, arraySize);
    for (int i = 0; i < arraySize; i++)
        output[i] = temp[i];
  }
}

/**
 * @brief Sets all LEDs in the ring to a single color.
 *
 * This function fills the provided output array with the specified color,
 * effectively setting all LEDs in the ring to the same color.
 *
 * @param color The color to set for each LED (of type CRGB).
 * @param output Pointer to the array of CRGB objects representing the LED ring.
 * @param arraySize The number of LEDs in the ring (size of the output array).
 */
void LedRingController::singleColorRing(uint32_t color, uint32_t* output, int arraySize){
  for (int i = 0; i < arraySize; i++)
    output[i] = color;
}

uint32_t LedRingController::blendColor(uint32_t c1, uint32_t c2, uint8_t t) {
    // Extract RGB components
    uint8_t r1 = (c1 >> 16) & 0xFF;
    uint8_t g1 = (c1 >> 8) & 0xFF;
    uint8_t b1 = c1 & 0xFF;

    uint8_t r2 = (c2 >> 16) & 0xFF;
    uint8_t g2 = (c2 >> 8) & 0xFF;
    uint8_t b2 = c2 & 0xFF;

    // Linear interpolation
    uint8_t r = r1 + ((r2 - r1) * t) / 255;
    uint8_t g = g1 + ((g2 - g1) * t) / 255;
    uint8_t b = b1 + ((b2 - b1) * t) / 255;

    return strip.Color(r, g, b);
}

uint8_t LedRingController::beatsin8(float bpm, uint8_t low, uint8_t high) {
    float beat = millis() / (60000.0 / bpm); // Convert BPM to period in ms
    float wave = (sin(beat * TWO_PI) + 1.0) / 2.0; // Normalize 0–1
    return low + (high - low) * wave;
}