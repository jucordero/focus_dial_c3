#include "PiezoController.h"
#include HW_CONFIG

PiezoController::PiezoController(int buzzerPin)
    : buzzerPin(buzzerPin),
      melodyRunning(false),
      currentNote(0),
      sound_level(sound_level)
      {}

void PiezoController::begin(uint8_t sound_level) {
    pinMode(buzzerPin, OUTPUT);
    this->sound_level = sound_level;
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

    if (sound_level > 0)
        tone(buzzerPin, melody.notes[currentNote], melody.durations[currentNote]);
}

void PiezoController::beep(int frequency, int duration){
    if (sound_level > 1)
        tone(buzzerPin, frequency, duration);
}

void PiezoController::stopMelody() {
    melodyRunning = false;
}

void PiezoController::updateMelody() {
    if (millis() - lastNoteTime >= melody.durations[currentNote]) {
        currentNote++;
        if (currentNote < melody.notes.size()) {
            if (sound_level > 0)
                tone(buzzerPin, melody.notes[currentNote]);
        } else {
            noTone(buzzerPin);
            if (melody.loop) {
                currentNote = 0;
                if (sound_level > 0)
                    tone(buzzerPin, melody.notes[currentNote], melody.durations[currentNote]);
            } else 
                melodyRunning = false;
        }
        lastNoteTime = millis();
    }
}   