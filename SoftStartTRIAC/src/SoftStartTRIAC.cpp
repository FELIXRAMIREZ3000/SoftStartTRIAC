
// SoftStartTRIAC.cpp
#include "SoftStartTRIAC.h"

SoftStartTRIAC::SoftStartTRIAC(uint8_t pinZeroCross, uint8_t pinTriac, uint8_t pinControl, uint8_t pinRelay, uint8_t pinLed)
  : pinZero(pinZeroCross), pinTriac(pinTriac), pinControl(pinControl), pinRelay(pinRelay), pinLed(pinLed),
    previousZeroState(LOW), rampCompleted(false), outputActive(false), previousControlState(HIGH),
    lastZeroTime(0), measuredPeriod(8333), delayOutput(7850), delayInitial(7850),
    delayMinimum(550), pulseWidth(400), rampStep(5), lastBlinkTime(0),
    minPeriod(8000), maxPeriod(8696), superCycles(10), cyclesRemaining(0) {}

void SoftStartTRIAC::begin() {
    pinMode(pinZero, INPUT);
    pinMode(pinTriac, OUTPUT);
    pinMode(pinControl, INPUT_PULLUP);
    pinMode(pinLed, OUTPUT);
    pinMode(pinRelay, OUTPUT);

    digitalWrite(pinTriac, HIGH); // TRIAC inactivo (MOC3021 apagado)
    digitalWrite(pinLed, LOW);
    digitalWrite(pinRelay, LOW);

    setupTimer();
}

void SoftStartTRIAC::setupTimer() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TCCR1B |= (1 << WGM12); // Modo CTC
    TCCR1B |= (1 << CS11);  // Prescaler 8
    sei();
}

void SoftStartTRIAC::update() {
    bool currentControl = digitalRead(pinControl);
    if (currentControl == LOW) {
        rampCompleted = false;
        outputActive = false;
        delayOutput = delayInitial;
        cyclesRemaining = 0;
        digitalWrite(pinTriac, HIGH);
        digitalWrite(pinLed, LOW);
        digitalWrite(pinRelay, LOW);
        return;
    }

    if (previousControlState == LOW && currentControl == HIGH) {
        rampCompleted = false;
        outputActive = false;
        delayOutput = delayInitial;
        cyclesRemaining = 0;
        digitalWrite(pinRelay, LOW);
    }

    previousControlState = currentControl;

    toggleLed(rampCompleted ? 500 : 100);
}

void SoftStartTRIAC::toggleLed(unsigned long intervalMs) {
    if (millis() - lastBlinkTime >= intervalMs) {
        lastBlinkTime = millis();
        digitalWrite(pinLed, !digitalRead(pinLed));
    }
}
