#include "SystemStatus.h"
#include "Battery.h"
#include "HardwarePins.h"
#include "Temperature.h"
#include "Motor.h"
#include "Globals.h"
#include <WiFi.h>
#include <Arduino.h>

SystemStatus SystemStatus_get() {
    SystemStatus s;

    s.batteryVoltage = Battery_getVoltage();
    s.temperatureC   = Temperature_getCelsius();

    s.doorState      = Motor_getState();
    s.openCycles     = Motor_getOpenCycles();
    s.closeCycles    = Motor_getCloseCycles();

    s.wifiConnected   = WiFi.isConnected();
    s.telegramEnabled = telegramEnabled;

    s.uptimeSeconds = millis() / 1000;

    return s;
}

String SystemStatus_getHealthLabel(const SystemStatus& s) {
    if (s.batteryVoltage < Battery_getCriticalVoltage())
        return "🔴 CRITICAL BATTERY";

    if (Temperature_isCritical())
        return "🔥 OVERHEAT";

    return "🟢 OK";
}

String SystemStatus_getHealthLabel() {
    return SystemStatus_getHealthLabel(SystemStatus_get());
}

void SystemStatus_updateHeartbeat() {
    static unsigned long lastBeat = 0;
    static bool ledState = false;

    if (millis() - lastBeat >= 500) {
        lastBeat = millis();
        ledState = !ledState;
        digitalWrite(PIN_HEARTBEAT, ledState);
    }
}
