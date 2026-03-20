#include <Arduino.h>
#include "StateController.h"
#include HW_CONFIG
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

  int DELTA_T_CW = EEPROM.readInt(EEPROM_DELTAT_CW_ADDR);
  int DELTA_T_CCW = EEPROM.readInt(EEPROM_DELTAT_CCW_ADDR);

  if (firstTime) {
    display.animation.start(bitmaps[previousPosition], 27, true);
    firstTime = false;
  }
  
  const char* ssidList1[3] = {"CocaYJuampi", "Manuel", "SKYIRTWD"};

  // Main page menu states
  const int NMODES = 6;

  const SystemState newCurrentState[] = {
      STATE_TIMER_SELECT,
      STATE_STOPWATCH_START,
      STATE_SETTINGS,
      STATE_PREPARE_SLEEP,
      STATE_PULSE_SELECT,
      STATE_WIFI_SELECT};

  const String newCurrentStateString[] = {
      "Timer",
      "Stopwatch",
      "Settings",
      "Deepsleep",
      "Pulse",
      "Connection"};

  const ledRingAnimation newAnimation[] = {
      LEDRING_MODE_SELECT,
      LEDRING_MODE_SELECT,
      LEDRING_MODE_SELECT,
      LEDRING_PREPARE_SLEEP,
      LEDRING_MODE_SELECT,
      LEDRING_MODE_SELECT,
      LEDRING_MODE_SELECT};

  // Settings page states
  const int NSETTINGS_PAGES = 4;

  const SystemState newCurrentStateSettings[] = {
      STATE_SETTINGS_AUDIO,
      STATE_SETTINGS_DISPLAY,
      STATE_SETTINGS_LEDRING,
      STATE_SETTINGS_TIMER};

  const String newSettingsPageString[] = {
      "Sound",
      "Display",
      "Ledring",
      "Timer"};

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
      newPosition = (newPosition + NMODES) % (NMODES);
      input.setPosition(newPosition);
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      display.animation.start(bitmaps[currentPosition], 27, true); // Start the animation for the selected mode
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      display.animation.stop();
      
      ledRing.startAnimation(newAnimation[currentPosition], currentPosition, initialTimer, currentPosition, 20);
      currentState = newCurrentState[currentPosition];
      Serial.print("Entering new menu: ");
      Serial.println(newCurrentStateString[currentPosition]);
      input.setPosition(0);
      currentPosition = input.getPosition();
    }
    break;

  // ----------------
  //      TIMER
  // ----------------
  case STATE_TIMER_SELECT:
    newPosition = input.getPosition();
    if (newPosition != currentPosition) {
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      currentTimer = abs(currentPosition) * (currentPosition < 0 ? DELTA_T_CCW : DELTA_T_CW);
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      if (currentPosition==0){
        Serial.println("Returning to main menu");
        // ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, currentPosition);
        currentState = STATE_MODE_SELECT;
        input.setPosition(0);
        display.animation.start(hourglass, 27, true);
        currentPosition = input.getPosition();
      }

      else {
        currentState = STATE_TIMER_RUN;
        display.animation.start(play_pause, 19); // Set the countdown animation to start
        initialTimer = currentTimer;
        ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition, 20);
        countdownTimer = millis();
        Serial.print("Timer started with: ");
        Serial.print(currentTimer);
        Serial.println(" miliseconds");
        // positionTimer = millis();
      }
    }

    if (input.lastAction == BUTTON_LONG_PRESS){
      buttonFeedback(piezo);

      if (currentPosition==0){
        Serial.println("Returning to main menu");
        // ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, currentPosition, 20);
        currentState = STATE_MODE_SELECT;
        input.setPosition(0);
        display.animation.start(hourglass, 27, true);
        currentPosition = input.getPosition();
      }

      else {
        Serial.println("Reseting timer value");
        currentPosition = 0;
        input.setPosition(0);
        currentTimer = 0;
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
      ledRing.startAnimation(LEDRING_FINISHED_TIMER, currentTimer, initialTimer, currentPosition, 20);
      display.animation.start(ring_alarm, 27, true); 
      piezo.startMelody(loopBeep);
      lastInteractionTimer = millis();
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_TIMER_PAUSED;
      display.animation.start(play_pause, 19, false, true); // Set the countdown animation to start
      ledRing.startAnimation(LEDRING_PAUSE_TIMER, currentTimer, initialTimer, currentPosition, 20);
    }
    break;

  // Pause timer
  case STATE_TIMER_PAUSED:  
    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_TIMER_RUN;
      display.animation.start(play_pause, 19);
      ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition, 20);
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
    newPosition = input.getPosition();

    if (input.lastAction == BUTTON_SHORT_PRESS || newPosition != currentPosition) {
      buttonFeedback(piezo);
      piezo.stopMelody();
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
    newPosition = input.getPosition();

    if (newPosition != currentPosition) {
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      currentTimer = abs(currentPosition) * (currentPosition < 0 ? DELTA_T_CCW : DELTA_T_CW);
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
    
      if (currentPosition==0){
        // ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, 4);
        currentState = STATE_MODE_SELECT;
        input.setPosition(4);
        currentPosition = input.getPosition();
        display.animation.start(pulse, 27, true);
      }

      else {
        currentState = STATE_PULSE_RUN;
        display.animation.start(play_pause, 19);
        initialTimer = currentTimer;
        ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition, 20);
        countdownTimer = millis();
        // positionTimer = millis();
      }      
    }

    if (input.lastAction == BUTTON_LONG_PRESS){
      buttonFeedback(piezo);

      if (currentPosition==0){
        Serial.println("Returning to main menu");
        // ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentTimer, initialTimer, currentPosition);
        currentState = STATE_MODE_SELECT;
        input.setPosition(4);
        currentPosition = input.getPosition();
        display.animation.start(pulse, 27, true);
      }

      else {
        Serial.println("Reseting timer value");
        currentPosition = 0;
        input.setPosition(0);
        currentTimer = 0;
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
      ledRing.startAnimation(LEDRING_PULSE_FLASH, 5);
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
    newPosition = input.getPosition();

    if (newPosition != currentPosition) {
      newPosition = (newPosition + 3) % 4; // Wrap around to 0-3
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      input.setPosition(newPosition);      
    }

    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      if (currentPosition==0){
        // ledRing.startAnimation(LEDRING_RETURN_MAIN_MENU, currentPosition, initialTimer, 5);
        currentState = STATE_MODE_SELECT;
        input.setPosition(5);
        currentPosition = input.getPosition();
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
      input.setPosition(1);
      currentPosition = input.getPosition();
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
      ledRing.startAnimation(LEDRING_PAUSE_TIMER, currentTimer, initialTimer, currentPosition, 20);
    }
    break;

  // Pause stopwatch
  case STATE_STOPWATCH_PAUSED:
    if (input.lastAction == BUTTON_SHORT_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_STOPWATCH_RUN;
      display.animation.start(play_pause, 19);
      ledRing.startAnimation(LEDRING_START_TIMER, currentTimer, initialTimer, currentPosition, 20);
      timeNow = millis(); // Reset timeNow to current time
    }

    if (input.lastAction == BUTTON_LONG_PRESS) {
      buttonFeedback(piezo);
      currentState = STATE_STOPWATCH_START;
      // display.animation.start(watch, 27, true);
      input.setPosition(0);
      currentPosition = input.getPosition();
      currentTimer = 0; // Reset stopwatch timer
    }
    break;

  // ----------------
  //      SETTINGS
  // ----------------
  case STATE_SETTINGS:
    newPosition = input.getPosition();

    if (newPosition != currentPosition)
    {
      newPosition = (newPosition + NSETTINGS_PAGES) % NSETTINGS_PAGES;
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      input.setPosition(newPosition);
    }

    if (input.lastAction == BUTTON_SHORT_PRESS)
    {
      currentState = newCurrentStateSettings[currentPosition];
      Serial.print("Entering new menu: ");
      Serial.println(newSettingsPageString[currentPosition]);
      switch (currentPosition)
      {
      case 0:
        input.setPosition(piezo.sound_level);
        break;
      
      case 1:
        input.setPosition(display.brightness/10);
        break;
      
      case 2:
        input.setPosition(ledRing.brightness/10);
        break;
      
      case 3:
        input.setPosition(0);
        break;
      
      default:
        break;
      }
      currentPosition = input.getPosition();
      buttonFeedback(piezo);
    }

    if (input.lastAction == BUTTON_LONG_PRESS)
    {
      currentState = STATE_MODE_SELECT;
      display.animation.start(bitmaps[2], 27, true);
      input.setPosition(2);
      buttonFeedback(piezo);
      currentPosition = input.getPosition();
    }
    break;

  case STATE_SETTINGS_AUDIO:
    newPosition = input.getPosition();
    if (newPosition != currentPosition)
    {
      if (newPosition < 0) newPosition = 2;
      if (newPosition > 2) newPosition = 0;
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      input.setPosition(newPosition);
    }

    if (input.lastAction == BUTTON_LONG_PRESS)
    {
      currentState = STATE_SETTINGS;
      piezo.sound_level= newPosition; // Toggle piezo mute state
      EEPROM.writeUChar(EEPROM_PIEZO_MUTE_ADDR, piezo.sound_level);
      EEPROM.commit();
      input.setPosition(0);
      currentPosition = input.getPosition();
    }
    break;

  case STATE_SETTINGS_DISPLAY:
    newPosition = input.getPosition();
    if (newPosition != currentPosition)
    {
      if (newPosition < 0) {
        newPosition = 0;
        ledRing.startAnimation(LEDRING_SETTINGS_LIMIT, 5);
      }
      if (newPosition > 25){
        newPosition = 25;
        ledRing.startAnimation(LEDRING_SETTINGS_LIMIT, 5);
      }
      currentPosition = newPosition;
      display.setBrightness(currentPosition*10);
      rotaryFeedback(piezo);
      input.setPosition(newPosition);
    }

    if (input.lastAction == BUTTON_LONG_PRESS)
    {
      currentState = STATE_SETTINGS;
      EEPROM.writeInt(EEPROM_SCREEN_BRIGHTNESS_ADDR, currentPosition*10);
      EEPROM.commit();
      input.setPosition(2);
      currentPosition = input.getPosition();
    }
    break;

  case STATE_SETTINGS_LEDRING:
    newPosition = input.getPosition();
    if (newPosition != currentPosition)
    {
      if (newPosition < 0) {
        newPosition = 0;
        ledRing.startAnimation(LEDRING_SETTINGS_LIMIT, 5);
      }
      if (newPosition > 10) {
        newPosition = 10;
        ledRing.startAnimation(LEDRING_SETTINGS_LIMIT, 5);
      }
      currentPosition = newPosition;
      ledRing.setBrightness(currentPosition*10);
      rotaryFeedback(piezo);
      input.setPosition(newPosition);
    }
    
    if (input.lastAction == BUTTON_LONG_PRESS)
    {
      currentState = STATE_SETTINGS;
      EEPROM.writeInt(EEPROM_LEDRING_BRIGHTNESS_ADDR, currentPosition*10);
      EEPROM.commit();
      input.setPosition(2);
      currentPosition = input.getPosition();
    }
    break;

  case STATE_SETTINGS_TIMER:
    newPosition = input.getPosition();
    if (newPosition != currentPosition)
    {
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      input.setPosition(newPosition);
    }

    if (input.lastAction == BUTTON_SHORT_PRESS)
    {
      if (currentPosition%2 == 0) {
        currentState = STATE_SETTINGS_TIMER_CCW;
        Serial.println("Editing CCW timer delta");
        input.setPosition(DELTA_T_CCW/1000);
        currentPosition = input.getPosition();
      }
      else {
        currentState = STATE_SETTINGS_TIMER_CW;
        Serial.println("Editing CW timer delta");
        input.setPosition(DELTA_T_CW/1000);
        currentPosition = input.getPosition();      
      }
    }

    if (input.lastAction == BUTTON_LONG_PRESS)
    {
      currentState = STATE_SETTINGS;
      input.setPosition(3);
      currentPosition = input.getPosition();
    }
    break;

  case STATE_SETTINGS_TIMER_CW:
    newPosition = input.getPosition();
    if (newPosition != currentPosition)
    {
      if (newPosition < 1) {
        newPosition = 1;
        ledRing.startAnimation(LEDRING_SETTINGS_LIMIT, 5);
      }
      if (newPosition > 10){
        newPosition = 10;
        ledRing.startAnimation(LEDRING_SETTINGS_LIMIT, 5);
      }
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      input.setPosition(newPosition);
    }

    if (input.lastAction == BUTTON_LONG_PRESS)
    {
      currentState = STATE_SETTINGS_TIMER;
      EEPROM.writeInt(EEPROM_DELTAT_CW_ADDR, currentPosition*1000);
      EEPROM.commit();
      input.setPosition(3);
      currentPosition = input.getPosition();
    }

    break;

  case STATE_SETTINGS_TIMER_CCW:
    newPosition = input.getPosition();
    if (newPosition != currentPosition)
    {
      if (newPosition < 10) {
        newPosition = 10;
        ledRing.startAnimation(LEDRING_SETTINGS_LIMIT, 5);
      }
      if (newPosition > 60) {
        newPosition = 60;
        ledRing.startAnimation(LEDRING_SETTINGS_LIMIT, 5);
      }
      currentPosition = newPosition;
      rotaryFeedback(piezo);
      input.setPosition(newPosition);
    }

    if (input.lastAction == BUTTON_LONG_PRESS)
    {
      currentState = STATE_SETTINGS_TIMER;
      EEPROM.writeInt(EEPROM_DELTAT_CCW_ADDR, currentPosition*1000);
      EEPROM.commit();
      input.setPosition(0);
      currentPosition = input.getPosition();
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
      input.setPosition(1);
      currentPosition = input.getPosition();
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

    // Add button wake-up condition
    #ifdef DEVICE_VARIANT_H2
      esp_sleep_enable_ext1_wakeup(1 << SWITCH_PIN, ESP_EXT1_WAKEUP_ANY_LOW);
    #endif

    #ifdef DEVICE_VARIANT_C3
      esp_deep_sleep_enable_gpio_wakeup(1 << SWITCH_PIN, ESP_GPIO_WAKEUP_GPIO_LOW);
    #endif

    // Go to sleep
    delay(50);
    esp_deep_sleep_start();
}

void StateController::buttonFeedback(PiezoController& piezo) {
  Serial.println("Button pressed");
  if (piezo.sound_level > 1) {
      piezo.beep(NOTE_A6, 50);
  }
  lastInteractionTimer = millis();
}

void StateController::rotaryFeedback(PiezoController& piezo) {
  Serial.print("Rotary encoder moved to position: ");
  Serial.println(currentPosition);
  if (piezo.sound_level > 1) {
      piezo.beep(NOTE_A6, 50);
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