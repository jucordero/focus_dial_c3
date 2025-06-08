#include "Melody.h"

Melody finishMelody = {
    {NOTE_C5, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, 0, NOTE_B4, NOTE_C5},
    {200, 100, 100, 200, 200, 200, 200, 200},
    false
};

Melody rotaryUpMelody = {
    {NOTE_A6},
    {50},
    false
};

Melody loopBeep = {
    {NOTE_A6, 0, NOTE_A6, 0},
    {50, 200, 50, 1000},
    true
};