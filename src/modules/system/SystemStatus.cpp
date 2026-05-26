#include "modules/system/SystemStatus.h"

#include "modules/battery/BatteryModule.h"
#include "modules/sensors/SensorModule.h"
#include "modules/motor/MotorTask.h"
#include "modules/scheduler/SchedulerTask.h"
#include "modules/automode/AutoModeTask.h"
#include "modules/system/Globals.h"
#include "modules/time/TimeManager.h"

#include <WiFi.h>

static String s_lastAction = "None";

// ---------------------------------------------------------
// Build a full system snapshot
// ---------------------------------------------------------
SystemStatus SystemStatus_get() {
    SystemStatus s;

    // Core metrics
    s.batteryVoltage = Battery_getSmoothedVoltage();
    s.temperatureC   = Sensor_getTemperature();
    s.doorState      = Motor_getState();

    // Motor cycles
    s.openCycles  = Motor_getOpenCycles();
    s.closeCycles = Motor_getCloseCycles();

    // System info
    s.wifiRSSI = WiFi.RSSI();
    s.uptimeSeconds = millis() / 1000;

    // Mode + override
    s.autoModeEnabled = !AutoMode_isOverrideActive();
    s.overrideActive  = remoteOverride;
    s.overrideUntil   = remoteOverrideUntil;

    // Scheduler
    s.nextOpen  = Scheduler_getNextOpen();
    s.nextClose = Scheduler_getNextClose();

    // Last action
    s.lastAction = s_lastAction;

    return s;
}

// ---------------------------------------------------------
// Health label
// ---------------------------------------------------------
String SystemStatus_getHealthLabel(const SystemStatus& s) {
    if (s.batteryVoltage < 11.2f) return "🔴 CRITICAL BATTERY";
    if (s.temperatureC > 60)      return "🔥 OVERHEAT";
    if (s.batteryVoltage < 12.0f) return "🟡 LOW BATTERY";
    return "🟢 OK";
}

// ---------------------------------------------------------
// Last action helpers
// ---------------------------------------------------------
String System_getLastActionString() {
    return s_lastAction;
}

void System_setLastAction(const String& action) {
    s_lastAction = action;
}
