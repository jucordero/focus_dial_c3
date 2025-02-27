#include <Arduino.h>
#include "Animation.h"

Animation::Animation(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* display) : u8g2(display), animationRunning(false), playInReverse(false) {}

/**
 * Starts the animation with the provided frames, duration, and other parameters.
 *
 * @param frames An array of bitmaps representing the animation frames.
 * @param frameCount The number of frames in the animation.
 * @param loop Whether the animation should loop.
 * @param reverse Whether the animation should play in reverse.
 * @param durationMs The duration of the animation in milliseconds. If 0, the duration is calculated based on the frame delay.
 * @param width The width of each animation frame.
 * @param height The height of each animation frame.
 */
void Animation::start(const unsigned char frames[][BITMAP_LENGTH], int frameCount, bool loop, bool reverse, unsigned long durationMs, int width, int height) {
    Serial.println("Starting animation");
    animationFrames = frames;
    totalFrames = frameCount;
    loopAnimation = loop;
    playInReverse = reverse; // Set reverse playback flag
    animationRunning = true;

    // Initialize current frame correctly based on direction
    currentFrame = playInReverse ? totalFrames - 1 : 0;
    frameWidth = width;
    frameHeight = height;
    frameDelay = DEFAULT_FRAME_DELAY;

    if (durationMs == 0) {
        animationDuration = totalFrames * frameDelay;
    } else {
        animationDuration = durationMs;
    }

    animationStartTime = millis();
    lastFrameTime = millis();

    frameX = (u8g2->getWidth() - frameWidth) / 2;
    frameY = (u8g2->getHeight() - frameHeight) / 2;

    u8g2->clearBuffer();
    u8g2->drawXBM(frameX, frameY, frameWidth, frameHeight, animationFrames[currentFrame]);
    u8g2->sendBuffer();
}

/**
 * Updates the animation by advancing to the next frame and displaying it.
 * This function is called periodically to update the animation.
 * It checks if the animation is running, and if so, it updates the current frame
 * based on the animation direction (forward or reverse) and the loop setting.
 * It then clears the display buffer, draws the current frame, and sends the buffer to the display.
 */
void Animation::update() {
    if (!animationRunning) return;

    unsigned long currentTime = millis();

    if (currentTime - animationStartTime >= animationDuration) {
        animationRunning = false;
        Serial.println("Animation finished");
        return;
    }

    // Check if it's time to advance to the next frame
    if (currentTime - lastFrameTime >= frameDelay) {
        lastFrameTime = currentTime;

        // Adjust current frame based on direction
        if (playInReverse) {
            currentFrame--;
            if (currentFrame < 0) { 
                if (loopAnimation) {
                    currentFrame = totalFrames - 1; // Wrap around to last frame
                } else {
                    animationRunning = false;
                    Serial.println("Animation finished");
                    return;
                }
            }
        } else {
            currentFrame++;
            if (currentFrame >= totalFrames) {
                if (loopAnimation) {
                    currentFrame = 0; // Wrap around to first frame
                } else {
                    animationRunning = false;
                    Serial.println("Animation finished");
                    return;
                }
            }
        }

        // Display the current frame
        u8g2->clearBuffer();
        u8g2->drawXBM(frameX, frameY, frameWidth, frameHeight, animationFrames[currentFrame]);
        u8g2->sendBuffer();

        
    }
}

/**
 * Checks if the animation is currently running.
 * 
 * @return true if the animation is running, false otherwise.
 */
bool Animation::isRunning() {
    return animationRunning;
}