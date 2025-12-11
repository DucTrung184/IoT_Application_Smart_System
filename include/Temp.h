#ifndef TEMP_H
#define TEMP_H

void initTemp();
void updateTempLogic();   // thay cho handleTemp()

extern float temperature;
extern float humidity;

extern bool fanState;
extern bool autoMode;     // true = auto, false = manual

void fanOn();
void fanOff();

#endif
