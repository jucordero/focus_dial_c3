#include "DisplayController.h"
#include "bitmaps.h"
#include <EEPROM.h>
#include HW_CONFIG

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

void DisplayController::begin(int){
    Wire.begin(pinSDA, pinSCL);
    u8g2.begin();
    // u8g2.setContrast(brightness);
}

/**
 * Updates the display based on the current system state and encoder value.
 * If an animation is running, it will be updated. Otherwise, the time screen
 * will be drawn based on the current state (TIME_SELECT or COUNTDOWN).
 *
 * @param state The current system state.
 * @param encoder The current value of the encoder, representing the time in seconds.
 */
void DisplayController::update(
    SystemState state,
    long int timer,
    long int position) {

    if (animation.isRunning())
    animation.update();
    
    else { //If animation has finished, resume normal operation.
        u8g2.clearBuffer();

        switch (state)
        {
        case STATE_TIMER_SELECT:
            drawTimeScreen(timer);
            break;
        
        case STATE_TIMER_RUN:
            drawTimeScreen(timer);
            break;

        case STATE_TIMER_PAUSED:
            drawTimeScreen(timer);
            break;

        case STATE_PULSE_SELECT:
            drawTimeScreen(timer);
            break;
        
        case STATE_PULSE_RUN:
            drawTimeScreen(timer);
            break;
            
        case STATE_STOPWATCH_START:
            drawTimeScreen(timer);
            break;

        case STATE_STOPWATCH_RUN:
            drawTimeScreen(timer);
            break;

        case STATE_STOPWATCH_PAUSED:
            drawTimeScreen(timer);
            break;

        // case STATE_MODE_SELECT:
        //     drawModeSelect(timer);
        //     break;

        case STATE_WIFI_SELECT:
            drawWifiSelect(timer);
            break;

        case STATE_SETTINGS:
            drawSettings(position);
            break;

        case STATE_SETTINGS_AUDIO:
            drawAudioSettings(position);
            break;

        case STATE_SETTINGS_DISPLAY:
            drawDisplaySettings(position);
            break;

        case STATE_SETTINGS_LEDRING:
            drawLedringSettings(position);
            break;

        case STATE_SETTINGS_TIMER:
            drawTimerSettings(position);
            break;

        case STATE_SETTINGS_TIMER_CW:
            drawTimerSettingsCW(position);
            break;

        case STATE_SETTINGS_TIMER_CCW:
            drawTimerSettingsCCW(position);
            break;

        case STATE_INFO:
            drawInfo();
            break;
            
        default:
            break;
        }
        drawBatteryLevel();

        if (digitalRead(SWITCH_PIN)==LOW){
            u8g2.setDrawColor(2);
            u8g2.drawBox(0,0,128,32);
            u8g2.setDrawColor(1);
        }
        
        u8g2.sendBuffer();
    }

}

/**
 * Draws the time screen on the display, displaying the current time value from the encoder.
 * The time is displayed in the format "MM:SS" and an icon is drawn in the top right corner.
 * 
 * @param encoder The current value of the encoder, representing the time in seconds.
 */
void DisplayController::drawTimeScreen(long int timer) {
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    long int toSeconds = timer/1000;

    long int mm = toSeconds / 60;
    long int ss = toSeconds % 60;

    char timeStr[6];
    u8g2.setFont(Fonts::LargeNumber);
    sprintf(timeStr, "%02ld:%02ld", mm, ss);
    u8g2.drawStr(0, 32, timeStr);

    char tensOfSecondsStr[2];
    sprintf(tensOfSecondsStr, "%1ld", (timer % 1000) / 100);
    u8g2.setFont(Fonts::MediumText);
    u8g2.drawStr(91, 32, tensOfSecondsStr);

    // u8g2.drawXBMP(113, 14, 15, 16, image_download_bits);

}

void DisplayController::drawWifiSelect(long int timer) {
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    u8g2.setFont(Fonts::MediumText);
    u8g2.setCursor(0, 20);

    switch (timer){
        case 0:
            u8g2.print(".. Back");
            break;
        case 1:
            u8g2.print("CocaYJuampi");
            break;
        case 2:
            u8g2.print("Manuel");
            break;
        case 3:
            u8g2.print("SKYIRTWD");
            break;

        default:
            break;
        }
}

void DisplayController::drawAudioSettings(long int position)
{
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    u8g2.setFont(Fonts::SmallText);
    u8g2.setCursor(0, 12);
    u8g2.print("Sound");
    
    u8g2.setFont(Fonts::MediumText);
    u8g2.setCursor(0, 32);

    switch (position)
    {
    case 0:
        u8g2.print("Muted");
        break;

    case 1:
        u8g2.print("Alarms");
        break;

    case 2:
        u8g2.print("All sounds");
        break;

    default:
        break;
    }

}

void DisplayController::drawDisplaySettings(long int position)
{
    String valueText = String(position * 10) + "%";
    drawSettingsText("Display brightness", valueText.c_str());
}

void DisplayController::drawLedringSettings(long int position)
{
    String valueText = String(position * 10) + "%";
    drawSettingsText("Ring brightness", valueText.c_str());
}

void DisplayController::drawTimerSettings(long int position)
{
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    u8g2.setFont(Fonts::SmallText);
    u8g2.setCursor(0, 12);
    u8g2.print("Timer settings");

    u8g2.setFont(Fonts::MediumText);
    u8g2.setCursor(0, 32);

    if (position%2 == 0)
        u8g2.print("CCW step");
    else
        u8g2.print("CW step");
    
}

void DisplayController::drawTimerSettingsCW(long int position)
{
    String valueText = String(position * 1000) + "ms";
    drawSettingsText("CW step", valueText.c_str());

}

void DisplayController::drawTimerSettingsCCW(long int position)
{
    String valueText = String(position * 1000) + "ms";
    drawSettingsText("CCW step", valueText.c_str());
}

void DisplayController::drawSettings(long int position)
{

    switch (position)
    {
    case 0:
        drawSettingsText("Settings", "Sound");
        break;
    case 1:
        drawSettingsText("Settings", "Display");
        break;
    case 2:
        drawSettingsText("Settings", "LED ring");
        break;
    case 3:
        drawSettingsText("Settings", "Timer");
        break;
    
    default:
        break;
    }
}

void DisplayController::drawInfo() {
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    char ssid[32]; // Buffer to store the SSID
    EEPROM.get(0, ssid); // Read the SSID from EEPROM starting at address 0

    u8g2.setFont(Fonts::MediumText);
    u8g2.setCursor(0, 20);
    u8g2.print("SSID:");
    u8g2.setCursor(0, 40);
    u8g2.print(ssid);

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

void DisplayController::drawBatteryLevel() {

    u8g2.setFont(Fonts::Symbols);
    switch (EEPROM.readUChar(EEPROM_PIEZO_MUTE_ADDR))
    {

    case 0:
        u8g2.drawGlyph(107, 21, 326); // Sound off icon
        break;
        
    case 1:
        u8g2.drawGlyph(107, 21, 433); // Alarms only icon
        break;

    case 2:
        u8g2.drawGlyph(107, 21, 484); // Sound on icon
        break;

    default:
        break;
    }

    int level = analogRead(BATTERY_PIN);
    int c_level = constrain(level, BATTERY_LOW_LEVEL, BATTERY_HIGH_LEVEL); 
    int batPctg = map(c_level, BATTERY_LOW_LEVEL, BATTERY_HIGH_LEVEL, 0, 100);

    u8g2.setFont(Fonts::SmallText);
    char batStr[5];
    snprintf(batStr, sizeof(batStr), "%d%%", batPctg);
    u8g2.drawStr(110, 32, batStr);
    // u8g2.setCursor(90, 32);
    // u8g2.print(level);

    // u8g2.setFont(u8g2_font_blipfest_07_tn);
    // char levelStr[6];
    // snprintf(levelStr, sizeof(levelStr), "%d", level);
    // u8g2.drawStr(110, 32, levelStr);

    // u8g2.drawXBM(115, 16, 12, 15, battery_base);
    // Serial.println(analogRead(BATTERY_PIN));

}

void DisplayController::drawSettingsText(const char* headText, const char* valueText) {
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);

    u8g2.setFont(Fonts::SmallText);
    u8g2.setCursor(0, 12);
    u8g2.print(headText);

    u8g2.setFont(Fonts::MediumText);
    u8g2.setCursor(0, 32);
    u8g2.print(valueText);

}