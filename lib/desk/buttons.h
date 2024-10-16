#pragma once
#include <Arduino.h>
#include <message.h>

#define BUTTON_LENGTH 2

extern byte BUTTON_UP[BUTTON_LENGTH];
extern byte BUTTON_DOWN[BUTTON_LENGTH];
extern byte BUTTON_NONE[BUTTON_LENGTH];
extern byte BUTTON_M1[BUTTON_LENGTH];
extern byte BUTTON_M2[BUTTON_LENGTH];
extern byte BUTTON_STAND[BUTTON_LENGTH];
extern byte BUTTON_SIT[BUTTON_LENGTH];
extern byte BUTTON_MEM[BUTTON_LENGTH];

extern Message NO_CMD;
extern Message UP_CMD;
extern Message DOWN_CMD;

extern Message M1_CMD;
extern Message M2_CMD;
extern Message M3_CMD;
extern Message M4_CMD;
extern Message SIT_CMD;
extern Message STAND_CMD;
