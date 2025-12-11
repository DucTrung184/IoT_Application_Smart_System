#include "Smoke.h"

Smoke::Smoke(uint8_t mq2Pin, uint8_t buzzerPin, int threshold)
    : _mq2Pin(mq2Pin), _buzzerPin(buzzerPin), _threshold(threshold)
{}

void Smoke::begin() {
    pinMode(_mq2Pin, INPUT);

    ledcSetup(_pwmChannel, _pwmFreq, _pwmResolution);
    ledcAttachPin(_buzzerPin, _pwmChannel);
    ledcWrite(_pwmChannel, 0);

    Serial.println("[Smoke] MQ-2 heating 3s...");
    delay(3000);
    Serial.println("[Smoke] Ready.");
}

void Smoke::update() {
    _gasValue = analogRead(_mq2Pin);

    if (_autoMode) {
        if (_gasValue > _threshold)
            beep(true);
        else
            beep(false);
    } 
    else {
        beep(_manualBuzzerState);
    }
}

void Smoke::beep(bool state) {
    if (!state) {
        ledcWriteTone(_pwmChannel, 0);
        return;
    }

    ledcWriteTone(_pwmChannel, 2000);
}

void Smoke::setAutoMode(bool state) {
    _autoMode = state;
}

void Smoke::manualBuzzerControl(bool state) {
    _manualBuzzerState = state;
}
