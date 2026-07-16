
#pragma once
#include <Arduino.h>

//Actual LiPo Logic
void batteryBegin(int capacity);
int batteryGetCharge();

//Shift Register Logic, for LED Charge % Matrix
void batteryIndicatorMatrixBegin(uint32_t DataIn, uint32_t Clock, uint32_t LatchClock, uint32_t OutputEnable);
void batteryIndicatorMatrixShowCharge();
void batteryIndicatorMatrixShowByte(uint8_t value);