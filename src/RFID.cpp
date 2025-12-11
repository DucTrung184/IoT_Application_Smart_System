#include "RFID.h"
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include "MqttManager.h"

#define RFID_SS   16
#define RFID_RST  19
#define RFID_SCK  17
#define RFID_MOSI 5
#define RFID_MISO 18

#define SERVO_PIN 15
#define PROX_PIN 22  

String VALID_UID = "15 E 9C 28";

MFRC522 mfrc522(RFID_SS, RFID_RST);
Servo doorServo;

bool doorOpenFlag = false;
unsigned long doorTimer = 0;
unsigned long doorStartTime = 0;
bool obstacleDetected = false;
unsigned long lastObstacleSeen = 0;

void doorOpen() {
    doorServo.write(90);
    doorOpenFlag = true;
    doorStartTime = millis();
    Serial.println("Door opened");
}

void doorClose() {
    doorServo.write(0);
    doorOpenFlag = false;
    Serial.println("Door closed");
}

bool isObstacle() {
    return digitalRead(PROX_PIN) == LOW;
}

void handleDoorWithSensorNonBlocking() {
    if (!doorOpenFlag) return;

    unsigned long now = millis();

    if (doorTimer == 0) {
        obstacleDetected = false;
        doorTimer = now;
    }

    if (now - doorTimer <= 5000) {
        if (isObstacle()) obstacleDetected = true;
        return;
    }

    if (!obstacleDetected) {
        doorClose();
        extern MqttDoorManager mqttDoor;
        mqttDoor.publishStatus("Cửa đã đóng");
        doorTimer = 0;
        return;
    }

    if (isObstacle()) {
        lastObstacleSeen = now;
    } else {
        if (now - lastObstacleSeen >= 3000) {
            doorClose();
            extern MqttDoorManager mqttDoor;
            mqttDoor.publishStatus("Cửa đã đóng");
            doorTimer = 0;
        }
    }
}

void mqttDoorOpen() {
    doorOpen();
    extern MqttDoorManager mqttDoor;
    mqttDoor.publishStatus("Cửa đã mở");
}

void mqttDoorClose() {
    doorClose();
    extern MqttDoorManager mqttDoor;
    mqttDoor.publishStatus("Cửa đã đóng");
}

void doorSystemInit() {
    Serial.println("Initializing Door+RFID module...");
    pinMode(PROX_PIN, INPUT);

    SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
    mfrc522.PCD_Init();
    Serial.println("RFID Ready");

    doorServo.attach(SERVO_PIN);
    doorClose();
}

void doorSystemLoop() {
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) uid += " ";
    }
    uid.toUpperCase();

    Serial.print("Card detected: ");
    Serial.println(uid);

    extern MqttDoorManager mqttDoor;

    if (uid == VALID_UID) {
        Serial.println("ACCESS GRANTED");
        mqttDoor.publishStatus("Chủ nhân về");
        doorOpen();
        doorTimer = 0;
    } else {
        Serial.println("ACCESS DENIED");
        mqttDoor.publishStatus("Có người cố mở cửa!");
    }

    mfrc522.PICC_HaltA();
}
