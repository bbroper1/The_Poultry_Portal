#include "modules/system/SystemStatus.h"
#include "modules/battery/BatteryModule.h"
#include "modules/sensors/SensorModule.h"
#include "modules/motor/MotorTask.h"

static String s_lastAction = "None";

SystemStatus SystemStatus_get() {
    SystemStatus s;

    // Match your actual module APIs
    s.batteryVoltage = Battery_getVoltage();
    s.temperatureC   = Sensor_getTemperature();
    s.doorState      = Motor_getState();
    s.openCycles     = Motor_getOpenCycles();
    s.closeCycles    = Motor_getCloseCycles();

    return s;
}

String SystemStatus_getHealthLabel(const SystemStatus& s) {
    if (s.batteryVoltage < 11.5) return "CRITICAL";
    if (s.batteryVoltage < 12.0) return "LOW";
    if (s.temperatureC > 60)     return "HOT";
    return "OK";
}

String System_getLastActionString() {
    return s_lastAction;
}

void System_setLastAction(const String& action) {
    s_lastAction = action;
}