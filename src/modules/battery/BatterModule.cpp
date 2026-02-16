#include "BatteryModule.h"

// Cached values
static float s_batteryVoltage = 12.4f;
static float s_batteryCurrent_mA = 0.0f;

void Battery_begin() {
    // Initialize INA219 or ADC here if needed
}

// ---------------------------------------------------------
// Setters (SensorTask updates these once per second)
// ---------------------------------------------------------
void Battery_setVoltage(float v) {
    s_batteryVoltage = v;
}

void Battery_setCurrentmA(float mA) {
    s_batteryCurrent_mA = mA;
}

// ---------------------------------------------------------
// Getters
// ---------------------------------------------------------
float Battery_getVoltage() {
    return s_batteryVoltage;
}

float Battery_getCurrentmA() {
    return s_batteryCurrent_mA;
}

// ---------------------------------------------------------
// Convert voltage → percent (simple curve)
// ---------------------------------------------------------
int Battery_getPercent() {
    float v = s_batteryVoltage;

    if (v >= 12.6f) return 100;
    if (v >= 12.4f) return 90;
    if (v >= 12.2f) return 80;
    if (v >= 12.0f) return 70;
    if (v >= 11.8f) return 60;
    if (v >= 11.6f) return 50;
    if (v >= 11.4f) return 40;
    if (v >= 11.2f) return 30;
    if (v >= 11.0f) return 20;
    return 10;
}

// ---------------------------------------------------------
// Safety helper
// ---------------------------------------------------------
bool Battery_isCritical() {
    return s_batteryVoltage < 11.2f;
}