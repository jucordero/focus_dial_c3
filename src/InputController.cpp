#include <Arduino.h>
#include "InputController.h"
#include "config.h"
#include <esp_sleep.h>
#include "Animation.h"
#include "bitmaps.h"
#include "Melody.h"
#include "melodies.h"

InputController::InputController(int pinEncoder1, int pinEncoder2, int pinButton)
    : pinEncoder1(pinEncoder1),
      pinEncoder2(pinEncoder2),
      pinButton(pinButton),
      currentState(STATE_TIME_SELECT),
      lastInteractionTimer(millis()),
      encoder(RotaryEncoder(pinEncoder1, pinEncoder2, RotaryEncoder::LatchMode::TWO03)) {}

void InputController::begin() {
    pinMode(pinButton, INPUT_PULLUP);
    bounce.attach(pinButton, INPUT_PULLUP);
    bounce.interval(5);

    // Retain the previous position value
    encoder.setPosition(previousPosition);  // Adjust based on your encoder scale
}

void InputController::update(DisplayController& display,
                             LedRingController& ledRing,
                             PiezoController& piezo) {
  bounce.update();
  encoder.tick();
  long newPosition;
  unsigned long int timeNow;
  unsigned long int timeElapsed;
  newPosition = encoder.getPosition();

  switch (currentState)
  { 
  case STATE_TIME_SELECT:
    if (newPosition != position) {
      Serial.print("New encoder position: ");
      Serial.println(newPosition);
      position = newPosition;
      lastInteractionTimer = millis();
      if (position < 0) timer = -position*DELTA_T_CCW;
      else timer = position*DELTA_T_CW;
      piezo.startMelody(rotaryUpMelody);
    }

    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_COUNTDOWN;
      display.animation.start(animation_tick, 20); // Set the countdown animation to start
      initialTimer = timer;
      ledRing.startAnimation(START_COUNTDOWN, timer, initialTimer, position);
      lastInteractionTimer = millis();
      countdownTimer = millis();
      positionTimer = millis();
    }

    if (millis() - lastInteractionTimer > SLEEP_TIMEOUT)
      enterDeepSleep(display, ledRing);

    break;

  case STATE_COUNTDOWN:
    timeNow = millis();
    timeElapsed = timeNow - countdownTimer;
    timer = timer - timeElapsed;
    countdownTimer = timeNow;

    if (timer < 0){
      currentState = STATE_COUNTDOWN_FINISHED;
      // position = 0;
      ledRing.startAnimation(COUNTDOWN_FINISHED, timer, initialTimer, position);
      piezo.startMelody(startupMelody);
      lastInteractionTimer = millis();
    }

    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_COUNTDOWN_PAUSED;
      display.animation.start(animation_tick, 20); // Set the countdown animation to start
      ledRing.startAnimation(PAUSE_COUNTDOWN, timer, initialTimer, position);
      lastInteractionTimer = millis();
    }
    break;

  case STATE_COUNTDOWN_PAUSED:
      if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_COUNTDOWN;
      display.animation.start(animation_tick, 20); // Set the countdown animation to start
      ledRing.startAnimation(START_COUNTDOWN, timer, initialTimer, position);
      lastInteractionTimer = millis();
      countdownTimer = millis();
    }
    break;
  
  case STATE_COUNTDOWN_FINISHED:
    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_TIME_SELECT;
      ledRing.animationRunning = false;
      position = newPosition;
      if (position < 0) timer = -position*DELTA_T_CCW;
      else timer = position*DELTA_T_CW;
      lastInteractionTimer = millis();
    }
    
    if (newPosition != position) {
      currentState = STATE_TIME_SELECT;
      ledRing.animationRunning = false;
      Serial.print("New encoder position: ");
      Serial.println(newPosition);
      position = newPosition;
      if (position < 0) timer = -position*DELTA_T_CCW;
      else timer = position*DELTA_T_CW;
      lastInteractionTimer = millis();
    }

    if (millis() - lastInteractionTimer > SLEEP_TIMEOUT)
      enterDeepSleep(display, ledRing);
      
    break;

  default:
    break;
  }  
}

SystemState InputController::getState() {
  return currentState;
}

long int InputController::getPosition() {
  return position;
}

long int InputController::getTimer() {
  return timer;
}

long int InputController::getInitialTimer() {
  return initialTimer;
}

void InputController::enterDeepSleep(DisplayController& display, LedRingController& ledRing) {
    Serial.println("Saving state and entering deep sleep...");

    display.sleepScreen();
    ledRing.LedRingSleep();

    previousState = currentState;
    previousPosition = position;
    // Configure GPIOs as wake-up sources
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    // Determine the current state of encoder pins
    int encoder1State = digitalRead(ENCODER_PIN1);
    int encoder2State = digitalRead(ENCODER_PIN2);

        // Configure GPIO wake-up conditions dynamically
    if (encoder1State == LOW)
        esp_deep_sleep_enable_gpio_wakeup(1 << ENCODER_PIN1, ESP_GPIO_WAKEUP_GPIO_HIGH);
    else
        esp_deep_sleep_enable_gpio_wakeup(1 << ENCODER_PIN1, ESP_GPIO_WAKEUP_GPIO_LOW);

    // if (encoder2State == LOW)
    //     esp_deep_sleep_enable_gpio_wakeup(1 << ENCODER_PIN2, ESP_GPIO_WAKEUP_GPIO_HIGH);
    // else
    //     esp_deep_sleep_enable_gpio_wakeup(1 << ENCODER_PIN2, ESP_GPIO_WAKEUP_GPIO_LOW);

    // Add button wake-up condition
    esp_deep_sleep_enable_gpio_wakeup(1 << ENCODER_SWITCH, ESP_GPIO_WAKEUP_GPIO_LOW);

    // esp_deep_sleep_enable_gpio_wakeup(1 << ENCODER_SWITCH, ESP_GPIO_WAKEUP_GPIO_LOW);

    // Go to deep sleep
    delay(50);
    esp_deep_sleep_start();
}