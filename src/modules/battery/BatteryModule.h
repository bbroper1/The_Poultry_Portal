#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Battery Module — voltage, current, smoothing, health
// ---------------------------------------------------------

// Initialize INA219 (or ADC)
void Battery_begin();

// Update from INA219 (called by SensorTask)
void Battery_updateFromINA219();

// Setters (override only)
void Battery_setVoltage(float v);
void Battery_setCurrentmA(float mA);

// Getters
float Battery_getVoltage();          // raw voltage
float Battery_getSmoothedVoltage();  // EMA smoothed voltage
float Battery_getCurrentmA();
int   Battery_getPercent();

// Daily min/max
float Battery_getMinToday();
float Battery_getMaxToday();

// Health
bool Battery_isValid();      // INA219 OK?
bool Battery_isCritical();   // raw voltage < threshold
bool Battery_isBrownout();   // smoothed voltage < threshold

// Smoothing control
void Battery_setVoltageSmoothing(float alpha);

// Optional: temperature read (if INA219 supports it)
float Battery_readTemperatureC();
