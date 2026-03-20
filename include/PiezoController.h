#ifndef PIEZO_CONTROLLER_H
#define PIEZO_CONTROLLER_H

#include "SystemState.h"
#include "Melody.h"

class PiezoController {
public:
    PiezoController(int buzzerPin);

    void begin(uint8_t sound_level = 0);
    void update(SystemState state);

    int buzzerPin;

    unsigned long int lastNoteTime;
    bool melodyRunning;
    void startMelody(Melody melody);
    void updateMelody();
    void stopMelody();
    void beep(int frequency, int duration);

    Melody melody;

    // 0 = muted, 1 = alarms only, 2 = all sounds
    uint8_t sound_level = 0;

    int currentNote;
};

#endif