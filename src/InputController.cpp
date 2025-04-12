#include <Arduino.h>
#include "InputController.h"
#include "config.h"
#include <esp_sleep.h>
#include "Animation.h"
#include "bitmaps.h"
#include "Melody.h"
#include "melodies.h"
#include <EEPROM.h>

InputController::InputController(int pinEncoder1, int pinEncoder2, int pinButton)
    : pinEncoder1(pinEncoder1),
      pinEncoder2(pinEncoder2),
      pinButton(pinButton),
      currentState(STATE_MODE_SELECT),
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
  long newPosition;
  unsigned long int timeNow;
  unsigned long int timeElapsed;
  const char* ssidList1[3] = {"CocaYJuampi", "Manuel", "SKYIRTWD"};
  const int NMODES = 6;

  switch (currentState)
  { 

  case STATE_PREPARE_SLEEP:
    if (millis() - lastInteractionTimer > 2000)
      enterDeepSleep(display, ledRing);
    break;

  // --------------
  // Mode selection
  // -------------- 
  case STATE_MODE_SELECT:    
    encoder.tick();
    newPosition = encoder.getPosition();

    if (newPosition != currentPosition) {
      Serial.print("New encoder position: ");
      if (newPosition < 0)
      newPosition = NMODES;
      else if (newPosition > NMODES)
      newPosition = 0;

      Serial.println(newPosition);
      encoder.setPosition(newPosition);
      currentPosition = newPosition;
      lastInteractionTimer = millis();
      piezo.startMelody(rotaryUpMelody);

      switch(currentPosition) {
        case 0:
          display.animation.start(hourglass, 27, true);
          break;
        case 1:
          display.animation.start(watch, 27, true);
          break;
        case 2:
          display.animation.start(gears, 27, true);
          break;
        case 3:
          display.animation.start(eye, 27, true);
          break;
        case 4:
          display.animation.start(pulse, 27, true);
          break;
        case 5:
          display.animation.start(wifi, 27, true);
          break;
        case 6:
          display.animation.start(info, 27, true);
          break;
      }
    }

    currentTimer = currentPosition;

    if (bounce.fell()) {
      ledRing.startAnimation(LEDRING_MODE_SELECT, currentTimer, initialTimer, currentPosition);
      Serial.print("Button pressed. Entering ");
      display.animation.stop();
      encoder.setPosition(0);
      piezo.startMelody(rotaryUpMelody);
      lastInteractionTimer = millis();
      currentPosition = encoder.getPosition();
      
      switch (currentTimer){
        case 0:
          Serial.println("timer.");
          currentState = STATE_TIMER_SELECT;
          currentTimer=0;
          break;

        case 1:
          Serial.println("stopwatch.");
          currentState = STATE_STOPWATCH_START;
          break;
        
        case 2:
          Serial.println("settings.");
          currentState = STATE_SETTINGS;
          break;

        case 3:
        Serial.println("sleep.");
          currentState = STATE_PREPARE_SLEEP;
          break;

        case 4:
        Serial.println("pulse.");
          currentState = STATE_PULSE_SELECT;
          break;

        case 5:
        Serial.println("wifi options.");
          currentState = STATE_WIFI_SELECT;
          break;

        case 6:
          Serial.println("info.");
          currentState = STATE_INFO;
          break;

        default:
          break;
      }
    }

    if (millis() - lastInteractionTimer > SLEEP_TIMEOUT)
      enterDeepSleep(display, ledRing);

    break;

  // ----------------
  //      TIMER
  // ----------------
  case STATE_TIMER_SELECT:
    encoder.tick();
    newPosition = encoder.getPosition();
    if (newPosition != currentPosition) {
      Serial.print("New encoder position: ");
      Serial.println(newPosition);
      currentPosition = newPosition;
      lastInteractionTimer = millis();
      if (currentPosition < 0) currentTimer = -currentPosition*DELTA_T_CCW;
      else currentTimer = currentPosition*DELTA_T_CW;
      piezo.startMelody(rotaryUpMelody);
    }

    if (bounce.fell()) {
      Serial.println("Button pressed");
      lastInteractionTimer = millis();

      if (currentPosition==0){
        ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, currentPosition);
        currentState = STATE_MODE_SELECT;
        encoder.setPosition(0);
      }
      else {
        currentState = STATE_TIMER_RUN;
        display.animation.start(play_pause, 19); // Set the countdown animation to start
        initialTimer = currentTimer;
        ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition);
        countdownTimer = millis();
        positionTimer = millis();
      }
    }

    if (millis() - lastInteractionTimer > SLEEP_TIMEOUT)
      enterDeepSleep(display, ledRing);

    break;

  case STATE_TIMER_RUN:  
    timeNow = millis();
    timeElapsed = timeNow - countdownTimer;
    currentTimer = currentTimer - timeElapsed;
    countdownTimer = timeNow;

    if (currentTimer < 0){
      currentState = STATE_TIMER_FINISHED;
      // position = 0;
      ledRing.startAnimation(LEDRING_FINISHED_TIMER, currentTimer, initialTimer, currentPosition);
      display.animation.start(ring_alarm, 27, true); 
      piezo.startMelody(loopBeep);
      lastInteractionTimer = millis();
    }

    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_TIMER_PAUSED;
      display.animation.start(play_pause, 19, false, true); // Set the countdown animation to start
      ledRing.startAnimation(LEDRING_PAUSE_TIMER, currentTimer, initialTimer, currentPosition);
      lastInteractionTimer = millis();
    }
    break;

  case STATE_TIMER_PAUSED:  
    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_TIMER_RUN;
      display.animation.start(play_pause, 19);
      ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition);
      lastInteractionTimer = millis();
      countdownTimer = millis();
    }
    break;
  
  case STATE_TIMER_FINISHED:
    encoder.tick();
    newPosition = encoder.getPosition();

    if (bounce.fell() || newPosition != currentPosition) {
      Serial.println("Button pressed");
      currentState = STATE_TIMER_SELECT;
      ledRing.animationRunning = false;
      display.animation.stop();
      currentPosition = newPosition;
      if (currentPosition < 0) currentTimer = -currentPosition*DELTA_T_CCW;
      else currentTimer = currentPosition*DELTA_T_CW;
      piezo.startMelody(rotaryUpMelody);
      lastInteractionTimer = millis();
    }

    if (millis() - lastInteractionTimer > SLEEP_TIMEOUT)
      enterDeepSleep(display, ledRing);
      
    break;


  // ----------------
  //      PULSE
  // ----------------
  case STATE_PULSE_SELECT:
    encoder.tick();
    newPosition = encoder.getPosition();

    if (newPosition != currentPosition) {
      Serial.print("New encoder position: ");
      Serial.println(newPosition);
      currentPosition = newPosition;
      lastInteractionTimer = millis();
      if (currentPosition < 0) currentTimer = -currentPosition*DELTA_T_CCW;
      else currentTimer = currentPosition*DELTA_T_CW;
      piezo.startMelody(rotaryUpMelody);
    }

    if (bounce.fell()) {
      Serial.println("Button pressed");
      lastInteractionTimer = millis();

      if (currentPosition==0){
        ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, currentPosition);
        currentState = STATE_MODE_SELECT;
        encoder.setPosition(4);
        currentPosition = encoder.getPosition();
        display.animation.start(pulse, 27, true);
        piezo.startMelody(rotaryUpMelody);
      }

      else {
        currentState = STATE_PULSE_RUN;
        display.animation.start(play_pause, 19);
        initialTimer = currentTimer;
        ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition);
        lastInteractionTimer = millis();
        countdownTimer = millis();
        positionTimer = millis();
        piezo.startMelody(rotaryUpMelody);
      }
      
    }

    if (millis() - lastInteractionTimer > SLEEP_TIMEOUT)
      enterDeepSleep(display, ledRing);

    break;

  case STATE_PULSE_RUN:
    timeNow = millis();
    timeElapsed = timeNow - countdownTimer;
    currentTimer = currentTimer - timeElapsed;
    countdownTimer = timeNow;

    if (currentTimer < 0){
      Serial.println("Pulse timer finished");
      currentTimer += initialTimer;
      piezo.startMelody(rotaryUpMelody);
    }

    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_PULSE_SELECT;
      if (currentPosition < 0) currentTimer = -currentPosition*DELTA_T_CCW;
      else currentTimer = currentPosition*DELTA_T_CW;
      display.animation.start(play_pause, 19, false, true);
      piezo.startMelody(rotaryUpMelody);
      lastInteractionTimer = millis();
    }

    break;

  // ----------------
  //      WIFI
  // ----------------
  
  case STATE_WIFI_SELECT:

    encoder.tick();
    newPosition = encoder.getPosition();

    if (newPosition != currentPosition) {
      Serial.print("New encoder position in wifi select: ");
      
      if (newPosition < 0)
      newPosition = 3;
      else if (newPosition > 3)
      newPosition = 0;
      
      Serial.println(newPosition);

      currentPosition = newPosition;
      encoder.setPosition(newPosition);
      
      lastInteractionTimer = millis();
      piezo.startMelody(rotaryUpMelody);
    }
    currentTimer = currentPosition;

    if (bounce.fell()) {
      Serial.println("Button pressed");
      lastInteractionTimer = millis();
      piezo.startMelody(rotaryUpMelody);

      if (currentPosition==0){
        ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, currentPosition);
        currentState = STATE_MODE_SELECT;
        encoder.setPosition(5);
        currentPosition = encoder.getPosition();
        display.animation.start(wifi, 27, true);
        EEPROM.commit();
      }

      else {
        EEPROM.writeString(EEPROM_SSID_ADDR, ssidList1[currentPosition-1]);
      }
    }

    if (millis() - lastInteractionTimer > SLEEP_TIMEOUT)
      enterDeepSleep(display, ledRing);

    break;

  // ----------------
  //      STOPWATCH
  // ----------------
  case STATE_STOPWATCH_START:
    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_MODE_SELECT;
      display.animation.start(watch, 19);
      lastInteractionTimer = millis();
      piezo.startMelody(rotaryUpMelody);
    }
    break;


  // ----------------
  //      SETTINGS
  // ----------------
  case STATE_SETTINGS:
    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_MODE_SELECT;
      display.animation.start(gears, 19);
      lastInteractionTimer = millis();
      piezo.startMelody(rotaryUpMelody);
    }
    break;


  // ----------------
  //      INFO
  // ----------------
  case STATE_INFO:
    if (bounce.fell()) {
      Serial.println("Button pressed");
      currentState = STATE_MODE_SELECT;
      display.animation.start(info, 19);
      lastInteractionTimer = millis();
      piezo.startMelody(rotaryUpMelody);
    }
    break;


  default:
    break;
  }  
}

SystemState InputController::getState() {
  return currentState;
}

long int InputController::getPosition() {
  return currentPosition;
}

long int InputController::getTimer() {
  return currentTimer;
}

long int InputController::getInitialTimer() {
  return initialTimer;
}

void InputController::enterDeepSleep(DisplayController& display, LedRingController& ledRing) {
    Serial.println("Saving state and entering deep sleep...");

    Serial.end();
    display.sleepScreen();
    ledRing.LedRingSleep();

    previousState = currentState;
    previousPosition = currentPosition;
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