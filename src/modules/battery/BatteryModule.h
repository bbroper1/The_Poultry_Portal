#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Battery Module — stores voltage + current + percent
// ---------------------------------------------------------

// Initialize battery subsystem (INA219 or ADC)
void Battery_begin();

// Setters (called by SensorTask)
void Battery_setVoltage(float v);
void Battery_setCurrentmA(float mA);

// Getters
float Battery_getVoltage();
float Battery_getCurrentmA();
int   Battery_getPercent();

// Safety helpers
bool Battery_isCritical();