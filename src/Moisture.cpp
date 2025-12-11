#include "Moisture.h"

Moisture::Moisture(uint8_t analogPin, uint8_t pumpPin,
                   int lowThres, int highThres)
    : aPin(analogPin), pPin(pumpPin),
      lowThreshold(lowThres), highThreshold(highThres),
      pumpState(false), moisture(0), mode("auto") {}

void Moisture::begin() {
    pinMode(pPin, OUTPUT);
    digitalWrite(pPin, HIGH);
}

void Moisture::setPump(bool on) {
    pumpState = on;
    digitalWrite(pPin, on ? LOW : HIGH);
}

void Moisture::setMode(const String &m) {
    mode = m;
}

void Moisture::update() {
    int adc = analogRead(aPin);

    moisture = map(adc, 50, 150, 100, 0);
    moisture = constrain(moisture, 0, 100);

    if (mode == "auto") {
        if (!pumpState && moisture < lowThreshold) setPump(true);
        else if (pumpState && moisture > highThreshold) setPump(false);
    }
}
