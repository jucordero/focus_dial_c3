#include <Arduino.h>
#include "DisplayController.h"
#include "InputController.h"
#include "LedRingController.h"
#include "PiezoController.h"
#include "config.h"
#include "utils.h"
#include "pitches.h"

SystemState currentState = STATE_TIME_SELECT;
DisplayController displayController(SDA_PIN, SCL_PIN);
InputController inputController(ENCODER_PIN1, ENCODER_PIN2, ENCODER_SWITCH);
LedRingController ledRingController(NUM_LEDS, LED_PIN, LED_BRIGHT);
PiezoController piezoController(BUZZER_PIN);

unsigned long tMemoryInfo = 0;

RTC_DATA_ATTR SystemState previousState = STATE_TIME_SELECT;
RTC_DATA_ATTR long int previousPosition = 0;

void setup(void) {

    Serial.begin(115200);

    // Check wake-up cause
    esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
    if (wakeupCause == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Wake-up triggered by EXT0!");
        currentState = previousState;
    } else {
        Serial.println("Cold start...");
        currentState = STATE_TIME_SELECT;
    }

  displayController.begin();
  inputController.begin();
  ledRingController.begin();
  piezoController.begin();
}

void loop(void) {

  inputController.update(displayController, ledRingController, piezoController);

  ledRingController.update(
    currentState = inputController.getState(),
    inputController.getPosition(),
    inputController.getTimer(),
    inputController.getInitialTimer()
    );
  
  displayController.update(
    currentState = inputController.getState(),
    inputController.getTimer()
    );

  piezoController.update(inputController.getState());
  
  if (millis() - tMemoryInfo > 5000) {
    tMemoryInfo = millis();
    printHeapInfo();
    printTaskStackInfo();
  }
}