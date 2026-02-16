#include "StatusMessage.h"
#include <WiFi.h>

// If you have real modules, include them here:
// #include "DoorModule.h"
// #include "EnergyModule.h"
// #include "BatteryModule.h"
// #include "SystemState.h"

String buildStatusMessage() {

    String msg;

    msg += "📊 *System Status*\n\n";

    // Door state (placeholder)
    msg += "Door: UNKNOWN\n";

    // Mode (placeholder)
    msg += "Mode: UNKNOWN\n";

    // WiFi RSSI
    msg += "WiFi: " + String(WiFi.RSSI()) + " dBm\n";

    // Uptime
    uint32_t ms = millis();
    uint32_t sec = ms / 1000;
    uint32_t min = sec / 60;
    uint32_t hr  = min / 60;
    uint32_t day = hr / 24;

    msg += "Uptime: " + String(day) + "d " +
           String(hr % 24) + "h " +
           String(min % 60) + "m\n";

    return msg;
}