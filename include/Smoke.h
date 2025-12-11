#ifndef SMOKE_H
#define SMOKE_H

#include <Arduino.h>

class Smoke {
public:
    Smoke(uint8_t mq2Pin, uint8_t buzzerPin, int threshold = 2100);

    void begin();
    void update();

    void setAutoMode(bool state);
    void manualBuzzerControl(bool state);

    int getGasValue() const { return _gasValue; }
    bool isAuto() const { return _autoMode; }

private:
    uint8_t _mq2Pin;
    uint8_t _buzzerPin;

    int _gasValue = 0;
    int _threshold;

    bool _autoMode = true;
    bool _manualBuzzerState = false;

    void beep(bool state);

    int _pwmChannel = 0;
    int _pwmFreq = 2000;
    int _pwmResolution = 8;
};

#endif
