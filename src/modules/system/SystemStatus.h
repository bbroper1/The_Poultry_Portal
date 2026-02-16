#pragma once
#include <Arduino.h>
#include "modules/motor/MotorTask.h"

// ---------------------------------------------------------
// Snapshot of system metrics for display + Telegram
// ---------------------------------------------------------
struct SystemStatus {
    float batteryVoltage;
    float temperatureC;
    MotorDoorState doorState;
    unsigned int openCycles;
    unsigned int closeCycles;
};

// Returns a snapshot of current system status
SystemStatus SystemStatus_get();

// Returns a human-readable health label
String SystemStatus_getHealthLabel(const SystemStatus& s);

// Last action helpers (your existing API)
String System_getLastActionString();
void   System_setLastAction(const String& action);