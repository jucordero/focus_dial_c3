#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "SystemState.h"

// OLED screen
#include <U8g2lib.h>
using DisplayType = U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C;
#define SDA_PIN 3
#define SCL_PIN 2

namespace Fonts {
static constexpr const uint8_t* LargeNumber = u8g2_font_logisoso32_tn;
static constexpr const uint8_t* MediumNumber = u8g2_font_logisoso16_tn;
}

// LED strip
#define LED_PIN  5
#define NUM_LEDS 24

// Encoder
#define ENCODER_PIN1 13
#define ENCODER_PIN2 14

// Switch
#define SWITCH_PIN 12
#define BUTTON_LONG_PRESS_THRESHOLD 1250

// Buzzer
#define BUZZER_PIN 11

// Battery
#define BATTERY_PIN 4
#define BATTERY_LOW_LEVEL 1600
#define BATTERY_HIGH_LEVEL 2100

// System config
#define SCREEN_TIME 10000
#define SLEEP_TIMEOUT 60000
extern RTC_DATA_ATTR SystemState previousState;
extern RTC_DATA_ATTR long int previousPosition;
#define HOLD_TIME 1000

// EEPROM
#define EEPROM_SIZE 512

#define EEPROM_SSID_SIZE 32
#define EEPROM_SSID_ADDR 0

#define EEPROM_PASSWORD_SIZE 32
#define EEPROM_PASSWORD_ADDR (EEPROM_SSID_ADDR + EEPROM_SSID_SIZE)

#define EEPROM_LEDRING_BRIGHTNESS_SIZE sizeof(int)
#define EEPROM_LEDRING_BRIGHTNESS_ADDR (EEPROM_PASSWORD_ADDR + EEPROM_PASSWORD_SIZE)

#define EEPROM_PIEZO_MUTE_SIZE sizeof(uint8_t)
#define EEPROM_PIEZO_MUTE_ADDR (EEPROM_LEDRING_BRIGHTNESS_ADDR + EEPROM_LEDRING_BRIGHTNESS_SIZE)

#define EEPROM_SCREEN_BRIGHTNESS_SIZE sizeof(int)
#define EEPROM_SCREEN_BRIGHTNESS_ADDR (EEPROM_PIEZO_MUTE_ADDR + EEPROM_PIEZO_MUTE_SIZE)

#define EEPROM_DELTAT_CW_SIZE sizeof(int)
#define EEPROM_DELTAT_CW_ADDR (EEPROM_SCREEN_BRIGHTNESS_ADDR + EEPROM_SCREEN_BRIGHTNESS_SIZE)

#define EEPROM_DELTATCCW_SIZE sizeof(int)
#define EEPROM_DELTAT_CCW_ADDR (EEPROM_DELTAT_CW_ADDR + EEPROM_DELTAT_CW_SIZE)

#endif