#ifndef ANIMATION_H
#define ANIMATION_H

#include <U8g2lib.h>
#include HW_CONFIG

#define DEFAULT_FRAME_WIDTH 64
#define DEFAULT_FRAME_HEIGHT 64
#define DEFAULT_FRAME_DELAY 42
#define BITMAP_LENGTH 512


class Animation
{
public:
    Animation(DisplayType *u8g2);
    void start(const unsigned char frames[][BITMAP_LENGTH], int frameCount, bool loop = false, bool reverse = false, unsigned long durationMs = 0, int width = DEFAULT_FRAME_WIDTH, int height = DEFAULT_FRAME_HEIGHT);
    void update();
    bool isRunning();
    void stop();

private:
    DisplayType *u8g2;
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
