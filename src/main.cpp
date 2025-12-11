#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "Temp.h"
#include "Smoke.h"
#include "RFID.h"
#include "Moisture.h"
#include "MqttManager.h"
#include "cert.h"

const char* WIFI_SSID  = "Haichiem";
const char* WIFI_PASS  = "04697297";

const char* MQTT_BROKER = "o06160ee.ala.asia-southeast1.emqxsl.com";
const int   MQTT_PORT   = 8883;
const char* MQTT_USER   = "esp32";
const char* MQTT_PASSW  = "1234567890";


MqttTempManager mqttTemp(MQTT_BROKER, MQTT_PORT, MQTT_USER, MQTT_PASSW, EMQX_ROOT_CA);
unsigned long lastPublishTemp = 0;


#define MQ2_PIN 34
#define BUZZER_PIN 14
Smoke smokeModule(MQ2_PIN, BUZZER_PIN, 1200);

MqttManager mqttSmoke(
    WIFI_SSID, WIFI_PASS,
    MQTT_BROKER, MQTT_PORT,
    MQTT_USER, MQTT_PASSW,
    &smokeModule
);
unsigned long lastPublishSmoke = 0;


MqttDoorManager mqttDoor(
    MQTT_BROKER,
    MQTT_PORT,
    MQTT_USER,
    MQTT_PASSW,
    EMQX_ROOT_CA
);


Moisture moistureModule(32, 25, 30, 60);

MqttMoistureManager mqttMoisture(
    MQTT_BROKER, MQTT_PORT,
    MQTT_USER, MQTT_PASSW,
    EMQX_ROOT_CA
);
unsigned long lastPublishMoisture = 0;


void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("Connecting WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWiFi Connected: " + WiFi.localIP().toString());
}


void setup() {
    Serial.begin(115200);
    delay(300);

    connectWiFi();


    initTemp();
    mqttTemp.begin();


    smokeModule.begin();
    mqttSmoke.begin();


    mqttDoor.begin();
    doorSystemInit();


    moistureModule.begin();
    mqttMoisture.begin();

    Serial.println("===== ALL MODULES READY =====");
}


void loop() {
    mqttTemp.loop();
    updateTempLogic();

    if (millis() - lastPublishTemp > 5005) {
        lastPublishTemp = millis();
        mqttTemp.publishTemp(temperature);
        mqttTemp.publishHumi(humidity);
        mqttTemp.publishFanState(fanState);
        mqttTemp.publishMode(autoMode);
    }

    mqttSmoke.loop();
    smokeModule.update();

    if (millis() - lastPublishSmoke > 500) {
        lastPublishSmoke = millis();
        mqttSmoke.publishSmokeData();
    }

    mqttDoor.loop();
    doorSystemLoop();
    handleDoorWithSensorNonBlocking();

    mqttMoisture.loop();
    moistureModule.update();

    if (millis() - lastPublishMoisture > 5000) {
        lastPublishMoisture = millis();
        mqttMoisture.publishMoisture(moistureModule.getMoisture());
        mqttMoisture.publishPumpState(moistureModule.getPumpState());
        mqttMoisture.publishMode(moistureModule.getMode());
    }
}
