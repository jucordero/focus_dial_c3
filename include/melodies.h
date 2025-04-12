#include "Melody.h"

Melody finishMelody = {
    {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4},
    {200, 100, 100, 200, 200, 200, 200, 200},
    false
};

Melody rotaryUpMelody = {
    {NOTE_A2},
    {50},
    false
};

Melody loopBeep = {
    {NOTE_A3, 0, NOTE_A3, 0},
    {50, 200, 50, 1000},
    true
};