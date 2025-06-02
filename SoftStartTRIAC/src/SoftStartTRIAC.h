
#ifndef SOFTSTART_TRIAC_H
#define SOFTSTART_TRIAC_H

#include <Arduino.h>

class SoftStartTRIAC {
public:
    SoftStartTRIAC(uint8_t pinZeroCross, uint8_t pinTriac, uint8_t pinControl, uint8_t pinRelay, uint8_t pinLed = LED_BUILTIN);

    void begin();
    void update();
    void handleZeroCross();
    void setupTimer();

    void resetRamp();
    bool isRampComplete();
    void setPulseWidthMicrosec(unsigned int us);
    void setInitialDelayMicrosec(unsigned int us);
    void setMinDelayMicrosec(unsigned int us);
    void setRampStepMicrosec(unsigned int us);
    void setSuperCycles(uint8_t count);

private:
    void triggerTriac(unsigned int delayMicrosec);
    void toggleLed(unsigned long intervalMs);

    uint8_t pinZero;
    uint8_t pinTriac;
    uint8_t pinControl;
    uint8_t pinRelay;
    uint8_t pinLed;

    bool previousZeroState;
    bool rampCompleted;
    bool outputActive;
    bool previousControlState;

    unsigned long lastZeroTime;
    unsigned long measuredPeriod;
    unsigned long delayOutput;
    unsigned long delayInitial;
    unsigned long delayMinimum;
    unsigned long pulseWidth;
    unsigned long rampStep;
    unsigned long lastBlinkTime;

    unsigned long minPeriod;
    unsigned long maxPeriod;

    uint8_t superCycles;
    int cyclesRemaining;
};

#endif
