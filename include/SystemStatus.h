#pragma once
#include <Arduino.h>
#include "Motor.h"

struct SystemStatus {
    float batteryVoltage;
    float temperatureC;

    MotorDoorState doorState;
    unsigned int openCycles;
    unsigned int closeCycles;

    bool wifiConnected;
    bool telegramEnabled;

    unsigned long uptimeSeconds;
};

SystemStatus SystemStatus_get();
String SystemStatus_getHealthLabel(const SystemStatus& s);
String SystemStatus_getHealthLabel();  
void SystemStatus_updateHeartbeat();
