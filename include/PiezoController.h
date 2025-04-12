#ifndef PIEZO_CONTROLLER_H
#define PIEZO_CONTROLLER_H

#include "SystemState.h"
#include "Melody.h"

class PiezoController {
public:
    PiezoController(int buzzerPin);

    void begin();
    void update(SystemState state);

    int buzzerPin;

    unsigned long int lastNoteTime;
    bool melodyRunning;
    void startMelody(Melody melody);
    void updateMelody();
    void stopMelody();  
    Melody melody;

    int currentNote;
};

#endif