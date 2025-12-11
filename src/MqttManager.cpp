#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "MqttManager.h"
#include "Temp.h"
#include "Smoke.h"
#include "RFID.h"
#include "Moisture.h"
#include "cert.h"

MqttTempManager* MqttTempManager::instance = nullptr;
MqttManager* MqttManager::instance = nullptr;
MqttDoorManager* MqttDoorManager::instance = nullptr;
MqttMoistureManager* MqttMoistureManager::instance = nullptr;

MqttTempManager::MqttTempManager(const char* broker, int port,
                                 const char* user, const char* pass,
                                 const char* caCert)
    : _broker(broker), _port(port), _user(user), _pass(pass),
      _caCert(caCert), _client(_wifiClient)
{
    instance = this;
}

void MqttTempManager::begin() {
    _wifiClient.setCACert(_caCert);
    _client.setServer(_broker, _port);
    _client.setCallback(MqttTempManager::mqttCallback);
}

void MqttTempManager::loop() {
    if (!_client.connected()) reconnect();
    _client.loop();
}

void MqttTempManager::reconnect() {
    while (!_client.connected()) {
        if (_client.connect("TempModule", _user, _pass)) {
            _client.subscribe("iot/temp/setFan");
            _client.subscribe("iot/temp/setMode");
            publishTemp(temperature);
            publishHumi(humidity);
            publishFanState(fanState);
            publishMode(autoMode);
        } else delay(2000);
    }
}

void MqttTempManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (!instance) return;
    String msg;
    for (uint32_t i = 0; i < length; i++) msg += (char)payload[i];
    instance->handleMessage(String(topic), msg);
}

void MqttTempManager::handleMessage(String topic, String msg) {
    if (topic == "iot/temp/setFan") {
        bool newState = (msg == "1");
        if (newState) fanOn(); else fanOff();
        autoMode = false;
        publishMode(false);
        publishFanState(fanState);
        return;
    }

    if (topic == "iot/temp/setMode") {
        bool newMode = (msg == "auto" || msg == "1");
        autoMode = newMode;
        publishMode(autoMode);

        if (autoMode) {
            if (temperature >= 30) fanOn();
            else if (temperature <= 25) fanOff();
        }
        publishFanState(fanState);
        return;
    }
}

void MqttTempManager::publishTemp(float temp) {
    _client.publish("iot/temp/value", String(temp).c_str(), true);
}

void MqttTempManager::publishHumi(float humi) {
    _client.publish("iot/temp/humi", String(humi).c_str(), true);
}

void MqttTempManager::publishFanState(bool state) {
    _client.publish("iot/temp/fanState", state ? "1" : "0", true);
}

void MqttTempManager::publishMode(bool mode) {
    _client.publish("iot/temp/mode", mode ? "auto" : "manual", true);
}

MqttManager::MqttManager(const char* ssid,
                         const char* password,
                         const char* broker,
                         int port,
                         const char* user,
                         const char* pass,
                         Smoke* smokeModule)
    : _ssid(ssid),
      _password(password),
      _broker(broker),
      _port(port),
      _mqttUser(user),
      _mqttPass(pass),
      client(espClient),
      _smoke(smokeModule)
{
    instance = this;
}

void MqttManager::begin() {
    WiFi.begin(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    espClient.setCACert(EMQX_ROOT_CA);
    client.setServer(_broker, _port);
    client.setCallback(MqttManager::mqttCallback);

    reconnect();
}

void MqttManager::loop() {
    if (!client.connected()) reconnect();
    client.loop();
}

void MqttManager::reconnect() {
    while (!client.connected()) {
        if (client.connect("ESP32_SMOKE", _mqttUser, _mqttPass)) {
            client.subscribe(TOPIC_SMOKE_MODE);
            client.subscribe(TOPIC_SMOKE_BUZZER);
        } else delay(2000);
    }
}

void MqttManager::publishSmokeData() {
    int val = _smoke->getGasValue();
    client.publish(TOPIC_SMOKE_VALUE, String(val).c_str(), true);
    client.publish(TOPIC_SMOKE_MODE, _smoke->isAuto() ? "auto" : "manual", true);
}

void MqttManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    String msg = String((char*)payload);

    if (String(topic) == "iot/smoke/mode") {
        instance->_smoke->setAutoMode(msg == "auto");
    } else if (String(topic) == "iot/smoke/buzzer") {
        instance->_smoke->setAutoMode(false);
        instance->client.publish("iot/smoke/mode", "manual", true);

        bool buzState = (msg == "on" || msg == "1");
        instance->_smoke->manualBuzzerControl(buzState);
    }
}

MqttDoorManager::MqttDoorManager(const char* broker, int port,
                                 const char* user, const char* pass,
                                 const char* caCert)
    : _broker(broker), _port(port),
      _user(user), _pass(pass),
      _caCert(caCert), _client(_wifiClient)
{
    instance = this;
}

void MqttDoorManager::begin() {
    _wifiClient.setCACert(_caCert);
    _client.setServer(_broker, _port);
    _client.setCallback(MqttDoorManager::mqttCallback);
}

void MqttDoorManager::loop() {
    if (!_client.connected()) reconnect();
    _client.loop();
}

void MqttDoorManager::reconnect() {
    while (!_client.connected()) {
        if (_client.connect("DoorModule", _user, _pass)) {
            _client.subscribe("iot/door/cmd");
            publishStatus("Door module online");
        } else delay(2000);
    }
}

void MqttDoorManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (!instance) return;

    String msg;
    for (uint32_t i = 0; i < length; i++) msg += (char)payload[i];

    instance->handleMessage(String(topic), msg);
}

void MqttDoorManager::handleMessage(String topic, String msg) {
    msg.toLowerCase();

    if (topic == "iot/door/cmd") {
        if (msg == "open") mqttDoorOpen();
        else if (msg == "close") mqttDoorClose();
    }
}

void MqttDoorManager::publishStatus(const String &msg) {
    _client.publish("iot/door/status", msg.c_str(), true);
}

extern Moisture moistureModule;

MqttMoistureManager::MqttMoistureManager(const char* broker, int port,
                                         const char* user, const char* pass,
                                         const char* caCert)
    : _broker(broker), _port(port), _user(user), _pass(pass),
      _caCert(caCert), _client(_wifiClient)
{
    instance = this;
}

void MqttMoistureManager::begin() {
    _wifiClient.setCACert(_caCert);
    _client.setServer(_broker, _port);
    _client.setCallback(MqttMoistureManager::mqttCallback);
}

void MqttMoistureManager::loop() {
    if (!_client.connected()) reconnect();
    _client.loop();
}

void MqttMoistureManager::reconnect() {
    while (!_client.connected()) {
        if (_client.connect("MoistureModule", _user, _pass)) {
            _client.subscribe("iot/moisture/setPump");
            _client.subscribe("iot/moisture/setMode");

            publishMoisture(moistureModule.getMoisture());
            publishPumpState(moistureModule.getPumpState());
            publishMode(moistureModule.getMode());
        } else delay(2000);
    }
}

void MqttMoistureManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (!instance) return;

    String msg;
    for (uint32_t i = 0; i < length; i++) msg += (char)payload[i];

    instance->handleMessage(String(topic), msg);
}

void MqttMoistureManager::handleMessage(String topic, String msg) {
    if (topic == "iot/moisture/setPump") {
        moistureModule.setPump(msg == "1");
        moistureModule.setMode("manual");
        publishMode("manual");
        publishPumpState(moistureModule.getPumpState());
        return;
    }

    if (topic == "iot/moisture/setMode") {
        String newMode = (msg == "auto" || msg == "1") ? "auto" : "manual";
        moistureModule.setMode(newMode);
        publishMode(newMode);

        if (newMode == "auto") {
            moistureModule.update();
            publishPumpState(moistureModule.getPumpState());
        }
        return;
    }
}

void MqttMoistureManager::publishMoisture(int value) {
    _client.publish("iot/moisture/value", String(value).c_str(), true);
}

void MqttMoistureManager::publishPumpState(bool state) {
    _client.publish("iot/moisture/pumpState", state ? "1" : "0", true);
}

void MqttMoistureManager::publishMode(const String &mode) {
    _client.publish("iot/moisture/mode", mode.c_str(), true);
}
