#include "HealthHandler.h"
#include "../../keyboards/TelegramKeyboards.h"

// System modules
#include "SystemStatus.h"
#include "BatteryModule.h"
#include "EnergyModule.h"
#include "MotorModule.h"
#include "SensorModule.h"
#include "TimeUtils.h"
#include "Config.h"

// Globals
#include "modules/system/Globals.h"

// ---------------------------------------------------------
// Formatting Helpers
// ---------------------------------------------------------

String HealthHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

String HealthHandler::kv(const String& label, const String& value) {
    return "• " + label + ": " + value + "\n";
}

String HealthHandler::formatTempLabel(float c) {
    if (c >= 40) return "🔥 HOT";
    if (c <= 0)  return "❄️ COLD";
    return "🙂 OK";
}

String HealthHandler::formatRemaining(time_t until) {
    time_t now = time(nullptr);
    if (until <= now) return "0m";

    int sec = until - now;
    int min = sec / 60;
    int hr  = min / 60;

    if (hr > 0)
        return String(hr) + "h " + String(min % 60) + "m";

    return String(min) + "m";
}

// ---------------------------------------------------------
// Main Handler
// ---------------------------------------------------------

void HealthHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    String kbd = kbMain();

    // Pull system snapshot
    SystemStatus s = SystemStatus_get();
    float cur = Motor_getCurrentmA();
    int pinch = Config_getPinchThreshold();
    esp_reset_reason_t reason = esp_reset_reason();

    float minV = Battery_getMinToday();
    float maxV = Battery_getMaxToday();

    // Build message
    String out;
    out.reserve(900);

    // HEADER
    out += blockHeader("🩺", "SYSTEM HEALTH");
    out += "\n";

    // WIFI
    out += blockHeader("📶", "WIFI");
    out += kv("RSSI", String(WiFi.RSSI()) + " dBm");
    out += "\n";

    // BATTERY
    out += blockHeader("🔋", "BATTERY");
    out += kv("Voltage", String(s.batteryVoltage, 2) + "V");
    out += kv("Today", String(maxV, 2) + "V max / " + String(minV, 2) + "V min");
    out += "\n";

    // MOTOR
    out += blockHeader("🔌", "MOTOR");
    out += kv("Current", String(cur, 1) + " mA");
    out += kv("Stall Threshold", String(pinch) + " mA");
    out += kv("Would Stall Now", (cur > pinch ? "YES 🚨" : "No"));

    float duty = Motor_getDutyCycle24h();
    uint32_t ms = Motor_getRuntimeMs24h();
    out += kv("Duty Cycle (24h)", String(duty, 2) + "% (" + String(ms / 1000) + " sec)");

    time_t st = Motor_getLastStallTime();
    if (st > 0) {
        String ts = TimeUtils::formatTimestamp(st);
        MotorDoorState dir = Motor_getLastStallDirection();
        String d = (dir == M_OPENING) ? "opening" :
                   (dir == M_CLOSING) ? "closing" : "unknown";
        out += kv("Last Stall", ts + " (" + d + ")");
    }
    out += "\n";

    // SYSTEM
    out += blockHeader("🖥️", "SYSTEM");
    out += kv("ESP Temp", String(s.temperatureC, 1) + "°C " + formatTempLabel(s.temperatureC));
    out += kv("Uptime", TimeUtils::getUptime());
    out += kv("Avg Daily", String(EnergySys_getAvgDailymAh(), 2) + " mAh/day");
    out += kv("Last 30 Days", String(EnergySys_getMonthlymAh(), 2) + " mAh");
    out += kv("Reset Reason", String((int)reason));
    out += kv("Free Heap", String(ESP.getFreeHeap()) + " bytes");
    out += kv("Min Heap", String(ESP.getMinFreeHeap()) + " bytes");
    out += kv("Health", SystemStatus_getHealthLabel(s));
    out += "\n";

    // TIME
    out += blockHeader("🕒", "TIME");

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday,
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec);
        out += kv("Local", String(buf));
    } else {
        out += kv("Local", "(not synced)");
    }

    out += kv("Last Daily Reset", EnergySys_getLastResetStr());
    out += "\n";

    // OVERRIDE
    if (remoteOverride) {
        out += blockHeader("🛠️", "OVERRIDE");
        out += kv("Status", "ACTIVE");
        out += kv("Time Left", formatRemaining(remoteOverrideUntil));
        out += kv("Ends At", TimeUtils::formatTimestamp(remoteOverrideUntil));
        out += "\n";
    }

    // SEND
    client->sendMessageWithKeyboard(evt.chatId, out, kbd);
}
