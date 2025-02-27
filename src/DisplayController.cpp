#include "DisplayController.h"
#include "bitmaps.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

/**
 * Constructs a DisplayController instance with the specified SDA and SCL pins.
 * The constructor initializes the u8g2 display object and the animation object.
 *
 * @param pinSDA The pin number for the SDA (Serial Data) line.
 * @param pinSCL The pin number for the SCL (Serial Clock) line.
 */
DisplayController::DisplayController(int pinSDA, int pinSCL)
    : pinSDA(pinSDA),
      pinSCL(pinSCL),
      u8g2(U8G2_R0, U8X8_PIN_NONE),
      animation(&u8g2)
      {}

void DisplayController::begin(){
    Wire.begin(pinSDA, pinSCL);
    u8g2.begin();
}

/**
 * Updates the display based on the current system state and encoder value.
 * If an animation is running, it will be updated. Otherwise, the time screen
 * will be drawn based on the current state (TIME_SELECT or COUNTDOWN).
 *
 * @param state The current system state.
 * @param encoder The current value of the encoder, representing the time in seconds.
 */
void DisplayController::update(SystemState state, long int timer) {
    if (animation.isRunning())
        animation.update();

    else { //If animation has finished, resume normal operation.
        switch (state)
        {
        case STATE_TIME_SELECT:
            drawTimeScreen(timer);
            break;
        case STATE_COUNTDOWN:
            drawTimeScreen(timer);
            break;
        case STATE_COUNTDOWN_PAUSED:
            drawTimeScreen(timer);
        
        default:
            break;
        }
    }
}

/**
 * Draws the time screen on the display, displaying the current time value from the encoder.
 * The time is displayed in the format "MM:SS" and an icon is drawn in the top right corner.
 * 
 * @param encoder The current value of the encoder, representing the time in seconds.
 */
void DisplayController::drawTimeScreen(long int timer) {
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    long int toSeconds = timer/1000;

    long int mm = toSeconds / 60;
    long int ss = toSeconds % 60;

    char timeStr[6];
    u8g2.setFont(u8g2_font_logisoso38_tn);
    sprintf(timeStr, "%02ld:%02ld", mm, ss);
    u8g2.drawStr(1, 51, timeStr);

    char tensOfSecondsStr[2];
    sprintf(tensOfSecondsStr, "%1ld", (timer % 1000) / 100);
    u8g2.setFont(u8g2_font_logisoso18_tn);
    u8g2.drawStr(112, 51, tensOfSecondsStr);

    u8g2.drawXBMP(113, 14, 15, 16, image_download_bits);
    u8g2.sendBuffer();
}

/**
 * Puts the display into a low-power sleep mode.
 * This clears the display buffer and sends the buffer to the display,
 * effectively turning off the display.
 */
void DisplayController::sleepScreen(){
    u8g2.setPowerSave(1);
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}
