#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <U8g2lib.h>
#include "SystemState.h"
#include "Animation.h"

class DisplayController {
public:
    DisplayController(int pinSDA, int pinSCL);

    void begin();
    void update(SystemState state, long int timer);

    void drawTimeScreen(long int encoder);
    void drawModeSelect(long int encoder);
    void drawWifiSelect(long int position);
    void drawSettings(long int position);
    void drawBatteryLevel();
    void drawInfo();
    void sleepScreen();

    int pinSCL;
    int pinSDA;

    Animation animation;

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
};

#endif