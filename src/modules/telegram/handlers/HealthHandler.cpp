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

void HealthHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    String kbd = kbMain();

    // Pull full system snapshot
    SystemStatus s = SystemStatus_get();

    // Temperature label (same logic as StatusHandler)
    String tempLabel;
    if (s.temperatureC >= 40) tempLabel = "🔥 HOT";
    else if (s.temperatureC <= 0) tempLabel = "❄️ COLD";
    else tempLabel = "🙂 OK";

    // Reset reason (ESP32)
    esp_reset_reason_t reason = esp_reset_reason();

    // Build message
    String out = "🩺 *SYSTEM HEALTH*\n━━━━━━━━━━━━━━━\n";

    out += "📶 WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    out += "🔋 Battery: " + String(s.batteryVoltage, 2) + "V\n";
    out += "⚡ Motor Current: " + String(Motor_getCurrentmA(), 0) + " mA\n";
    out += "🌡️ ESP Temp: " + String(s.temperatureC, 1) + "°C " + tempLabel + "\n";
    out += "⏱️ Uptime: " + TimeUtils::getUptime() + "\n";
    out += "📈 Avg Daily: " + String(Energy_getAvgDailymAh(), 1) + " mAh/day\n";
    out += "📅 Last 30 Days: " + String(Energy_getMonthlymAh(), 1) + " mAh\n";
    out += "🔁 Last Reset Reason: " + String((int)reason) + "\n";
    out += "💾 Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    out += "📉 Min Heap: " + String(ESP.getMinFreeHeap()) + " bytes\n";
    out += "🧠 Health: " + SystemStatus_getHealthLabel(s) + "\n\n";

    // Current local time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday,
                 timeinfo.tm_hour,
                 timeinfo.tm_min);

        out += "📅 Current Time: ";
        out += buf;
        out += "\n";
    } else {
        out += "📅 Current Time: (not synced)\n";
    }

    out += "🔄 Last Daily Reset: ";
    out += Energy_getLastResetStr();
    out += "\n";

    // Send
    client->sendMessageWithKeyboard(
        evt.chatId,
        out,
        kbd
    );
}