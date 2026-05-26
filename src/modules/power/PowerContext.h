#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// PowerContext — legacy global telemetry values
// ---------------------------------------------------------
// These globals are maintained by SensorModule, MotorController,
// and other subsystems. They are kept for backward compatibility
// but should be migrated into proper modules over time.
// ---------------------------------------------------------

// Battery voltage (input side)
extern float inputVoltage;

// Regulated output voltage (5V rail)
extern float outputVoltage;

// Average motor travel times (legacy)
extern float avgOpenTime;
extern float avgCloseTime;

// Critical battery cutoff threshold
extern const float CRITICAL_VOLTAGE;

// INA219 status + last measured motor current
extern bool  inaOK;
extern float motorCurrent;

// Optional: initialize defaults
void PowerContext_begin();
