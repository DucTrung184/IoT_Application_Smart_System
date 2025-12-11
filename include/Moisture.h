#pragma once
#include <Arduino.h>

class Moisture {
public:
    Moisture(uint8_t analogPin, uint8_t pumpPin,
             int lowThres = 30, int highThres = 60);

    void begin();
    void update();

    void setPump(bool on);
    void setMode(const String &mode);

    int getMoisture() const { return moisture; }
    bool getPumpState() const { return pumpState; }
    String getMode() const { return mode; }

private:
    uint8_t aPin;
    uint8_t pPin;

    int moisture;
    bool pumpState;

    int lowThreshold;
    int highThreshold;

    String mode;
};

extern Moisture moistureModule;
