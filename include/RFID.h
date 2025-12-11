#pragma once
#include <Arduino.h>

void doorSystemInit();
void doorSystemLoop();

void mqttDoorOpen();
void mqttDoorClose();

void handleDoorWithSensorNonBlocking();
