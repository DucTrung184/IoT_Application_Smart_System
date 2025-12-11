#include "Temp.h"
#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
#define RELAY_FAN_PIN 13

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;

bool fanState = false;
bool autoMode = true;  

void fanOn() {
    digitalWrite(RELAY_FAN_PIN, LOW);  
    fanState = true;
}

void fanOff() {
    digitalWrite(RELAY_FAN_PIN, HIGH);  
    fanState = false;
}

void initTemp() {
    dht.begin();
    pinMode(RELAY_FAN_PIN, OUTPUT);
    fanOff();
}

void updateTempLogic() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity = h;

    Serial.print("[TEMP] T = ");
    Serial.print(temperature);
    Serial.print(" °C  |  H = ");
    Serial.print(humidity);
    Serial.print(" %  | Fan: ");
    Serial.print(fanState ? "ON" : "OFF");
    Serial.print(" | Mode: ");
    Serial.println(autoMode ? "AUTO" : "MANUAL");

    if (autoMode) {
        if (temperature >= 30 && !fanState) {
            fanOn();
        }
        else if (temperature <= 25 && fanState) {
            fanOff();
        }
    }
}
