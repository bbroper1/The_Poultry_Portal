#pragma once
#include <Arduino.h>
#include <time.h>
#include "modules/motor/MotorTask.h"

// ---------------------------------------------------------
// Snapshot of system metrics for display + Telegram
// ---------------------------------------------------------
struct SystemStatus {
    // Core metrics
    float batteryVoltage;      // smoothed
    float temperatureC;        // smoothed
    MotorDoorState doorState;

    // Motor cycles
    unsigned int openCycles;
    unsigned int closeCycles;

    // System info
    int wifiRSSI;
    unsigned long uptimeSeconds;

    // Mode + override
    bool autoModeEnabled;
    bool overrideActive;
    time_t overrideUntil;

    // Scheduler
    String nextOpen;
    String nextClose;

    // Last action
    String lastAction;
};

// Returns a snapshot of current system status
SystemStatus SystemStatus_get();

// Returns a human-readable health label
String SystemStatus_getHealthLabel(const SystemStatus& s);

// Last action helpers
String System_getLastActionString();
void   System_setLastAction(const String& action);
