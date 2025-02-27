#ifndef ANIMATION_H
#define ANIMATION_H

#include <U8g2lib.h>

#define DEFAULT_FRAME_WIDTH 30
#define DEFAULT_FRAME_HEIGHT 30
#define DEFAULT_FRAME_DELAY 42
#define BITMAP_LENGTH 120

class Animation
{
public:
    Animation(U8G2_SSD1306_128X64_NONAME_F_HW_I2C *u8g2);
    void start(const unsigned char frames[][BITMAP_LENGTH], int frameCount, bool loop = false, bool reverse = false, unsigned long durationMs = 0, int width = DEFAULT_FRAME_WIDTH, int height = DEFAULT_FRAME_HEIGHT);
    void update();
    bool isRunning();

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C *u8g2;
    const unsigned char (*animationFrames)[BITMAP_LENGTH];
    int totalFrames;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    int frameX;
    int frameY;
    bool animationRunning;
    bool loopAnimation;
    bool playInReverse;
    unsigned long animationStartTime;
    unsigned long lastFrameTime;
    unsigned long animationDuration;
    unsigned long frameDelay;
};

#endif
