#include <Arduino.h>
#include "StateController.h"
#include "config.h"
#include <esp_sleep.h>
#include "Animation.h"
#include "bitmaps.h"
#include "Melody.h"
#include "melodies.h"
#include <EEPROM.h>

// Constructor
StateController::StateController()
    : currentState(STATE_MODE_SELECT),
      lastInteractionTimer(millis()) {}

// Update the state controller
void StateController::update(DisplayController& display,
                             LedRingController& ledRing,
                             PiezoController& piezo,
                             InputController& input) {

  if (firstTime) {
    display.animation.start(bitmaps[previousPosition], 27, true); // Start the animation for the previous position
    firstTime = false;
  }
  
  const char* ssidList1[3] = {"CocaYJuampi", "Manuel", "SKYIRTWD"};
  const int NMODES = 6;

  switch (currentState){ 

  // ----------------
  //      SLEEP
  // ----------------
  case STATE_PREPARE_SLEEP:
    if (millis() - lastInteractionTimer > 2000)
      enterDeepSleep(display, ledRing);
    break;

  // --------------
  // Mode selection
  // -------------- 
  case STATE_MODE_SELECT:
    newPosition = input.getPosition();
    if (newPosition != currentPosition) {
      rotaryFeedback(piezo);
      newPosition = (newPosition + NMODES + 1) % (NMODES + 1);
      input.encoder.setPosition(newPosition);
      currentPosition = newPosition;
      display.animation.start(bitmaps[currentPosition], 27, true); // Start the animation for the selected mode
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      display.animation.stop();
      
      const SystemState newCurrentState[] = {
        STATE_TIMER_SELECT, 
        STATE_STOPWATCH_START, 
        STATE_SETTINGS, 
        STATE_PREPARE_SLEEP, 
        STATE_PULSE_SELECT, 
        STATE_WIFI_SELECT, 
        STATE_INFO
      };
        
      const ledRingAnimation newAnimation[] = {
        LEDRING_MODE_SELECT, 
        LEDRING_MODE_SELECT, 
        LEDRING_MODE_SELECT, 
        LEDRING_PREPARE_SLEEP, 
        LEDRING_MODE_SELECT, 
        LEDRING_MODE_SELECT, 
        LEDRING_MODE_SELECT
      };
        
      ledRing.startAnimation(newAnimation[currentPosition], currentPosition, initialTimer, currentPosition);
      currentState = newCurrentState[currentPosition];      
      input.encoder.setPosition(0);
      currentPosition = input.encoder.getPosition();
    }
    break;

  // ----------------
  //      TIMER
  // ----------------
  case STATE_TIMER_SELECT:
    newPosition = input.encoder.getPosition();
    if (newPosition != currentPosition) {
      rotaryFeedback(piezo);
      currentPosition = newPosition;
      currentTimer = abs(currentPosition) * (currentPosition < 0 ? DELTA_T_CCW : DELTA_T_CW);
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      if (currentPosition==0){
        ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, currentPosition);
        currentState = STATE_MODE_SELECT;
        input.encoder.setPosition(0);
        display.animation.start(hourglass, 27, true);
        currentPosition = input.encoder.getPosition();
      }

      else {
        currentState = STATE_TIMER_RUN;
        display.animation.start(play_pause, 19); // Set the countdown animation to start
        initialTimer = currentTimer;
        ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition);
        countdownTimer = millis();
        // positionTimer = millis();
      }
    }
    break;

  // Start timer
  case STATE_TIMER_RUN:  
    timeNow = millis();
    timeElapsed = timeNow - countdownTimer;
    currentTimer = currentTimer - timeElapsed;
    countdownTimer = timeNow;
    Serial.print("Current Timer: ");
    Serial.println(currentTimer);

    if (currentTimer < 0){
      currentState = STATE_TIMER_FINISHED;
      ledRing.startAnimation(LEDRING_FINISHED_TIMER, currentTimer, initialTimer, currentPosition);
      display.animation.start(ring_alarm, 27, true); 
      piezo.startMelody(loopBeep);
      lastInteractionTimer = millis();
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_TIMER_PAUSED;
      display.animation.start(play_pause, 19, false, true); // Set the countdown animation to start
      ledRing.startAnimation(LEDRING_PAUSE_TIMER, currentTimer, initialTimer, currentPosition);
    }
    break;

  // Pause timer
  case STATE_TIMER_PAUSED:  
    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_TIMER_RUN;
      display.animation.start(play_pause, 19);
      ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition);
      countdownTimer = millis();
    }

    if (input.lastAction == BUTTON_LONG_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_TIMER_SELECT;
      currentTimer = abs(currentPosition) * (currentPosition < 0 ? DELTA_T_CCW : DELTA_T_CW);
    }
    break;
  
  // Timer finished
  case STATE_TIMER_FINISHED:
    newPosition = input.encoder.getPosition();

    if (input.lastAction == BUTTON_SHORT_PRESS || newPosition != currentPosition) {
      buttonFeedback(piezo);
      currentState = STATE_TIMER_SELECT;
      ledRing.animationRunning = false;
      display.animation.stop();
      currentPosition = newPosition;
      currentTimer = abs(currentPosition) * (currentPosition < 0 ? DELTA_T_CCW : DELTA_T_CW);
    }
    break;

  // ----------------
  //      PULSE
  // ----------------
  case STATE_PULSE_SELECT:
    newPosition = input.encoder.getPosition();

    if (newPosition != currentPosition) {
      rotaryFeedback(piezo);
      currentPosition = newPosition;
      currentTimer = abs(currentPosition) * (currentPosition < 0 ? DELTA_T_CCW : DELTA_T_CW);
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
    
      if (currentPosition==0){
        ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, 4);
        currentState = STATE_MODE_SELECT;
        input.encoder.setPosition(4);
        currentPosition = input.encoder.getPosition();
        display.animation.start(pulse, 27, true);
      }

      else {
        currentState = STATE_PULSE_RUN;
        display.animation.start(play_pause, 19);
        initialTimer = currentTimer;
        ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition);
        countdownTimer = millis();
        // positionTimer = millis();
      }      
    }
    break;

  // Run pulse timer
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

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_PULSE_SELECT;
      currentTimer = abs(currentPosition) * (currentPosition < 0 ? DELTA_T_CCW : DELTA_T_CW);
      display.animation.start(play_pause, 19, false, true);
    }
    break;

  // ----------------
  //      WIFI
  // ----------------
  case STATE_WIFI_SELECT:
    newPosition = input.encoder.getPosition();

    if (newPosition != currentPosition) {
      rotaryFeedback(piezo);
      newPosition = (newPosition + 3) % 4; // Wrap around to 0-3
      currentPosition = newPosition;
      input.encoder.setPosition(newPosition);      
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      if (currentPosition==0){
        ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentPosition, initialTimer, 5);
        currentState = STATE_MODE_SELECT;
        input.encoder.setPosition(5);
        currentPosition = input.encoder.getPosition();
        display.animation.start(wifi, 27, true);
        EEPROM.commit();
      }
      else EEPROM.writeString(EEPROM_SSID_ADDR, ssidList1[currentPosition-1]);
    }
    break;

  // ----------------
  //      STOPWATCH
  // ----------------
  case STATE_STOPWATCH_START:
    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_STOPWATCH_RUN;
      currentTimer = 0; // Reset stopwatch timer
      timeNow = millis();
    }

    if (input.lastAction == BUTTON_LONG_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_MODE_SELECT;
      display.animation.start(watch, 27, true);
      input.encoder.setPosition(1);
      currentPosition = input.encoder.getPosition();
    }
    break;

  // Run stopwatch
  case STATE_STOPWATCH_RUN:
    timeElapsed = millis() - timeNow;
    currentTimer += timeElapsed; // Update stopwatch timer
    timeNow = millis();
    Serial.print("Current Stopwatch Timer: ");
    Serial.println(currentTimer);

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_STOPWATCH_PAUSED;
      display.animation.start(play_pause, 19, false, true);
      ledRing.startAnimation(LEDRING_PAUSE_TIMER, currentTimer, initialTimer, currentPosition);
    }
    break;

  // Pause stopwatch
  case STATE_STOPWATCH_PAUSED:
    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_STOPWATCH_RUN;
      display.animation.start(play_pause, 19);
      ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition);
      timeNow = millis(); // Reset timeNow to current time
    }

    if (input.lastAction == BUTTON_LONG_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_STOPWATCH_START;
      // display.animation.start(watch, 27, true);
      input.encoder.setPosition(0);
      currentPosition = input.encoder.getPosition();
      currentTimer = 0; // Reset stopwatch timer
    }
    break;

  // ----------------
  //      SETTINGS
  // ----------------
  case STATE_SETTINGS:
    newPosition = input.encoder.getPosition();
    
    if (newPosition != currentPosition) {
      rotaryFeedback(piezo);
      currentPosition = newPosition;
      input.encoder.setPosition(newPosition);
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_MODE_SELECT;
      piezo.muted = newPosition%2 == 0; // Toggle piezo mute state
      EEPROM.write(EEPROM_PIEZO_MUTE_ADDR, piezo.muted ? 1 : 0);
      EEPROM.commit();
      display.animation.start(gears, 27, true);
      input.encoder.setPosition(2);
      currentPosition = input.encoder.getPosition();
    }
    break;

  // ----------------
  //      INFO
  // ----------------
  case STATE_INFO:
    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_MODE_SELECT;
      display.animation.start(info, 27, true);
      input.encoder.setPosition(6);
      currentPosition = input.encoder.getPosition();
    }
    break;

  default:
    break;
  }
  checkDeepSleep(display, ledRing, currentState);
}

SystemState StateController::getState() {
  return currentState;
}

long int StateController::getPosition() {
  return currentPosition;
}

long int StateController::getTimer() {
  return currentTimer;
}

long int StateController::getInitialTimer() {
  return initialTimer;
}

void StateController::enterDeepSleep(DisplayController& display, LedRingController& ledRing) {
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
    esp_deep_sleep_enable_gpio_wakeup(1 << SWITCH_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);
    // esp_deep_sleep_enable_gpio_wakeup(1 << ENCODER_SWITCH, ESP_GPIO_WAKEUP_GPIO_LOW);

    // Go to deep sleep
    delay(50);
    esp_deep_sleep_start();
}

void StateController::buttonFeedback(PiezoController& piezo) {
  Serial.println("Button pressed");
  if (!piezo.muted) {
      piezo.startMelody(rotaryUpMelody);
  }
  lastInteractionTimer = millis();
}

void StateController::rotaryFeedback(PiezoController& piezo) {
  Serial.println("Rotary encoder moved");
  if (!piezo.muted) {
      piezo.startMelody(rotaryUpMelody);
  }
  lastInteractionTimer = millis();
}

void StateController::checkDeepSleep(DisplayController& display, LedRingController& ledRing, SystemState state) {

  const SystemState deepSleepStates[] = {
    STATE_MODE_SELECT,
    STATE_TIMER_SELECT,
    STATE_TIMER_FINISHED,
    STATE_PULSE_SELECT,
    STATE_WIFI_SELECT,
    STATE_STOPWATCH_START,
    STATE_SETTINGS,
    STATE_INFO
  };

  if (millis() - lastInteractionTimer > SLEEP_TIMEOUT) {
    // Check if the current state is one of the deep sleep states
    for (SystemState deepSleepState : deepSleepStates) {
      if (state == deepSleepState) {
        Serial.println("Entering deep sleep due to inactivity");
        enterDeepSleep(display, ledRing);
      }
    }
  }
}