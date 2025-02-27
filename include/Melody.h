#ifndef MELODY_H
#define MELODY_H

#include <vector>
#include <Arduino.h>
#include "pitches.h"

struct Melody {
    std::vector<int> notes;
    std::vector<int> durations;
};

#endif