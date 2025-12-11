#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "Smoke.h"
#include "cert.h"

class MqttTempManager {
public:
    MqttTempManager(const char* broker, int port,
                    const char* user, const char* pass,
                    const char* caCert);

    void begin();
    void loop();

    void publishTemp(float temp);
    void publishHumi(float humi);
    void publishFanState(bool state);
    void publishMode(bool mode);

private:
    WiFiClientSecure _wifiClient;
    PubSubClient _client;

    const char* _broker;
    int _port;
    const char* _user;
    const char* _pass;
    const char* _caCert;

    static MqttTempManager* instance;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    void handleMessage(String topic, String msg);

    void reconnect();
};

class MqttManager {
public:
    MqttManager(const char* ssid,
                const char* password,
                const char* broker,
                int port,
                const char* user,
                const char* pass,
                Smoke* smokeModule);

    void begin();
    void loop();
    void publishSmokeData();

private:
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    void reconnect();

    static MqttManager* instance;

    const char* _ssid;
    const char* _password;
    const char* _broker;
    int _port;
    const char* _mqttUser;
    const char* _mqttPass;

    WiFiClientSecure espClient;
    PubSubClient client;

    Smoke* _smoke;

    const char* TOPIC_SMOKE_VALUE   = "iot/smoke/value";
    const char* TOPIC_SMOKE_MODE    = "iot/smoke/mode";
    const char* TOPIC_SMOKE_BUZZER  = "iot/smoke/buzzer";
};

class MqttDoorManager {
public:
    MqttDoorManager(const char* broker, int port,
                    const char* user, const char* pass,
                    const char* caCert);

    void begin();
    void loop();

    // Publish
    void publishStatus(const String &msg);

private:
    WiFiClientSecure _wifiClient;
    PubSubClient _client;

    const char* _broker;
    int _port;
    const char* _user;
    const char* _pass;
    const char* _caCert;

    static MqttDoorManager* instance;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    void handleMessage(String topic, String msg);

    void reconnect();
};

class MqttMoistureManager {
public:
    MqttMoistureManager(const char* broker, int port,
                        const char* user, const char* pass,
                        const char* caCert);

    void begin();
    void loop();

    void publishMoisture(int value);
    void publishPumpState(bool state);
    void publishMode(const String &mode);

private:
    WiFiClientSecure _wifiClient;
    PubSubClient _client;

    const char* _broker;
    int _port;
    const char* _user;
    const char* _pass;
    const char* _caCert;

    static MqttMoistureManager* instance;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    void handleMessage(String topic, String msg);

    void reconnect();
};
