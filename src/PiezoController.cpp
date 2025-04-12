#include "PiezoController.h"
#include "config.h"

PiezoController::PiezoController(int buzzerPin)
    : buzzerPin(buzzerPin),
      melodyRunning(false),
      currentNote(0)
      {}

void PiezoController::begin() {
    pinMode(buzzerPin, OUTPUT);
}

void PiezoController::update(SystemState state) {
    if (melodyRunning)
        updateMelody();
}

void PiezoController::startMelody(Melody melody) {
    this->melody = melody;
    melodyRunning = true;
    lastNoteTime = millis();
    currentNote = 0;
    tone(buzzerPin, melody.notes[currentNote], melody.durations[currentNote]);
}

void PiezoController::stopMelody() {
    melodyRunning = false;
}

void PiezoController::updateMelody() {
    if (millis() - lastNoteTime >= melody.durations[currentNote]) {
        currentNote++;
        if (currentNote < melody.notes.size()) {
            tone(buzzerPin, melody.notes[currentNote]);
        } else {
            noTone(buzzerPin);
            if (melody.loop) {
                currentNote = 0;
                tone(buzzerPin, melody.notes[currentNote], melody.durations[currentNote]);
            } else 
                melodyRunning = false;
        }
        lastNoteTime = millis();
    }
}   