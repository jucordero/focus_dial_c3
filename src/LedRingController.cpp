#include "LedRingController.h"
#include "FastLED.h"
#include "config.h"

LedRingController::LedRingController(int numLeds, int ledPin, int brightness)
  : numLeds(numLeds),
    ledPin(ledPin),
    brightness(brightness)
{
    leds = new CRGB[numLeds];
}

void LedRingController::begin() {
    FastLED.setBrightness(brightness);
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, numLeds);
    FastLED.clear();
    FastLED.show();
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
    case STATE_MODE_SELECT:
      LedRingModeSelect(abs(encoder));
      break;

    default:
      break;
    }
  }
}

void LedRingController::startAnimation(ledRingAnimation anim, long int timer, long int initialTimer, long int encoder){
  Serial.println("Starting LED animation");
  animationRunning = true;
  animation = anim;
  currentFrame = 0;
  animationStartTime = millis();

  // Populate initialState with the current LED state
  for (int i = 0; i < numLeds; i++) {
    initialState[i] = leds[i];
    endState[i] = CRGB::Black; // Default endState is off (black)
  }

  switch (anim)
  {
  case LEDRING_PAUSE_TIMER:
    totalFrames = 100;
    // assign single color ring in counting state to endState
    singleColorRingCounting(timer, initialTimer, encoder < 0, CRGB::Yellow, endState);
    break;

  case LEDRING_START_TIMER:
    totalFrames = 100;
    // assign single color ring in counting state to endState
    singleColorRingCounting(timer, initialTimer, encoder < 0, CRGB::Red, endState);
    break;

  case LEDRING_FINISHED_TIMER:
    // assign single color ring initialState and endState
    singleColorRing(CRGB::Black, initialState);
    singleColorRing(CRGB::White, endState);
    break;

  case LEDRING_RETURN_MAIN_MENU:
    totalFrames = 10;  
    for (int i = abs(encoder)%8*2; i < abs(encoder)%8*2+2; i++){
      endState[i] = CRGB::Aqua;
    }
    break;

  case LEDRING_PREPARE_SLEEP:
    totalFrames = 20;
    // assign single color ring in counting state to endState
    singleColorRing(CRGB::Black, endState);
    break;

  case LEDRING_MODE_SELECT:
    totalFrames = 10;
    singleColorRing(CRGB::Black, endState);
    break;


  default:
    break;
  }
}

void LedRingController::updateAnimation(){
  // Serial.print("Current frame: ");
  // Serial.println(currentFrame);

  if (millis() - lastFrameTime < frameDelay){
    Serial.println("Frame delay not reached");
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

    FastLED.clear();
    if (encoder < 0)
      singleColorRingSelecting(timer/DELTA_T_CCW, true, CRGB::Blue, newLeds);
    if (encoder >= 0)
      singleColorRingSelecting(timer/DELTA_T_CW, false, CRGB::Green, newLeds);
    for (int i = 0; i < numLeds; i++)
      leds[i] = newLeds[i];
    FastLED.show();
}

void LedRingController::LedRingTimeCountdown(long int timer, long int initialTimer, long int encoder){
    
    FastLED.clear();
    singleColorRingCounting(timer, initialTimer, encoder<0,  CRGB::Red, newLeds);
    for (int i = 0; i < numLeds; i++)
      leds[i] = newLeds[i];
    FastLED.show();
}

void LedRingController::LedRingCountdownPaused(long int timer, long int initialTimer, long int encoder){
    
    FastLED.clear();
    singleColorRingCounting(timer, initialTimer, encoder<0, CRGB::Yellow, newLeds);
    for (int i = 0; i < numLeds; i++)
      leds[i] = newLeds[i];
    FastLED.show();
}

void LedRingController::LedRingSleep(){
    FastLED.clear();
    FastLED.show();
}


void LedRingController::fadeBetweenStates(CRGB* initialState, CRGB* endState){
  Serial.print("Fading between states, current frame: ");
  Serial.println(currentFrame);
  FastLED.clear();

  float t = (float) currentFrame / totalFrames;
  for (int i = 0; i < numLeds; i++)
    leds[i] = blend(initialState[i], endState[i], t*255);
  currentFrame++;
  if (currentFrame > totalFrames)
    animationRunning = false;
  FastLED.show();
}

void LedRingController::pulseBetweenStates(CRGB* initialState, CRGB* endState){
  FastLED.clear();
  for (int i = 0; i < numLeds; i++)
    leds[i] = blend(initialState[i], endState[i], beatsin8(20, 0, 255));

  FastLED.show();
}

void LedRingController::LedRingModeSelect(long int encoder){
  FastLED.clear();
  unsigned int initPos = abs(encoder)%8*2;
  
  for (int i = initPos; i < initPos+2; i++){
    leds[i] = CRGB::Aqua;
  }
  FastLED.show();
}

int LedRingController::timeScale(long int encoder){
  int testExp = 1;
  while (true){
    if (encoder < 16*testExp)
      return testExp;
    else
      testExp *= 2;
  }    
}

void LedRingController::invertRing(const CRGB* ring, CRGB* output, int arraySize) {
  for (int i = 0; i < arraySize; i++)
      output[i] = ring[arraySize - i - 1];
}

void LedRingController::singleColorRingSelecting(long int timer, bool reversed, CRGB color, CRGB* output, int arraySize) {
  for (int i = 0; i < arraySize; i++)
    output[i] = CRGB::Black;

  int d = timeScale(timer);
  int posLeds = timer / d;
  int remLeds = timer % d;

  for (int i = 0; i < posLeds; i++)
    output[i] = color;

  output[posLeds] = blend(CRGB::Black, color, remLeds * 255 / d);

  if (reversed) {
      CRGB temp[arraySize];
      invertRing(output, temp, arraySize);
      for (int i = 0; i < arraySize; i++)
          output[i] = temp[i];
    }
}

void LedRingController::singleColorRingCounting(long int timer, long int initialTimer, bool reversed, CRGB color, CRGB* output, int arraySize) {
  for (int i = 0; i < arraySize; i++)
    output[i] = CRGB::Black;

  long int elapsed = (timer * 160) / initialTimer;

  int posLeds = elapsed / 10;
  int remLeds = elapsed % 10;

  for(int i = 0; i < posLeds; i++)
    output[i] = color;

  output[posLeds] = blend(CRGB::Black, color, remLeds * 255 / 10);

  if (reversed) {
    CRGB temp[arraySize];
    invertRing(output, temp, arraySize);
    for (int i = 0; i < arraySize; i++)
        output[i] = temp[i];
  }
}

void LedRingController::singleColorRing(CRGB color, CRGB* output, int arraySize){
  for (int i = 0; i < arraySize; i++)
    output[i] = color;
}

