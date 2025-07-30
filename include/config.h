#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "SystemState.h"

// OLED screen
#define SDA_PIN  8
#define SCL_PIN  9

// LED strip
#define LED_PIN  21 
#define NUM_LEDS  16
#define LED_BRIGHT 64

// Encoder
#define ENCODER_PIN1 3
#define ENCODER_PIN2 4

// Switch
#define SWITCH_PIN 2
#define BUTTON_LONG_PRESS_THRESHOLD 1250

// Buzzer
#define BUZZER_PIN 1

// Battery
#define BATTERY_PIN 0

// System config
#define SCREEN_TIME 10000
#define SLEEP_TIMEOUT 60000
extern RTC_DATA_ATTR SystemState previousState;
extern RTC_DATA_ATTR long int previousPosition;
#define HOLD_TIME 1500
#define DELTA_T_CW 10000
#define DELTA_T_CCW 60000

// EEPROM
#define EEPROM_SIZE 512

#define EEPROM_SSID_SIZE 32
#define EEPROM_SSID_ADDR 0
#define EEPROM_PASSWORD_SIZE 32
#define EEPROM_PASSWORD_ADDR (EEPROM_SSID_ADDR + EEPROM_SSID_SIZE)
#define EEPROM_BRIGHTNESS_SIZE sizeof(int)
#define EEPROM_BRIGHTNESS_ADDR (EEPROM_PASSWORD_ADDR + EEPROM_PASSWORD_ADDR)
#define EEPROM_PIEZO_MUTE_SIZE sizeof(bool)
#define EEPROM_PIEZO_MUTE_ADDR (EEPROM_BRIGHTNESS_ADDR + EEPROM_BRIGHTNESS_SIZE)

#endif