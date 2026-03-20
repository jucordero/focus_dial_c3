#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <U8g2lib.h>
#include "SystemState.h"
#include "Animation.h"
#include HW_CONFIG

class DisplayController {
public:
    DisplayController(int pinSDA, int pinSCL);

    void begin(int brightness = 128);
    void update(SystemState state, long int timer, long int position);

    void drawTimeScreen(long int encoder);
    // void drawModeSelect(long int encoder);
    void drawWifiSelect(long int position);
    void drawAudioSettings(long int position);
    void drawDisplaySettings(long int position);
    void drawLedringSettings(long int position);
    void drawTimerSettings(long int position);
    void drawSettings(long int position);
    void drawSettingsText(const char* headText, const char* valueText);
    void drawTimerSettingsCW(long int position);
    void drawTimerSettingsCCW(long int position);
    void drawBatteryLevel();
    void drawInfo();
    void sleepScreen();

    void setBrightness(int brightness){
        this->brightness = brightness;
        u8g2.setContrast(brightness);
    }

    int pinSCL;
    int pinSDA;

    int brightness;

    Animation animation;

private:
        DisplayType u8g2;
};

#endif