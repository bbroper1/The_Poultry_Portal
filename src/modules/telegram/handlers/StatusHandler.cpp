#include "StatusHandler.h"
#include "../../keyboards/TelegramKeyboards.h"
#include "../../automode/AutoModeTask.h"

// System modules
#include "SystemStatus.h"
#include "EnergyModule.h"
#include "SchedulerTask.h"
#include "Config.h"
#include "TimeUtils.h"
#include "../../time/TimeManager.h"

// ---------------------------------------------------------
// Formatting Helpers
// ---------------------------------------------------------

String StatusHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

String StatusHandler::kv(const String& label, const String& value) {
    return "• " + label + ": " + value + "\n";
}

String StatusHandler::formatDoorState(int state) {
    switch (state) {
        case M_OPEN:    return "OPEN ✅";
        case M_CLOSED:  return "CLOSED 🌙";
        case M_OPENING: return "OPENING ⚙️";
        case M_CLOSING: return "CLOSING ⚙️";
        case M_STUCK:   return "STUCK ⚠️";
    }
    return "UNKNOWN";
}

String StatusHandler::formatTempLabel(float c) {
    if (c >= 40) return "🔥 HOT";
    if (c <= 0)  return "❄️ COLD";
    return "🙂 OK";
}

String StatusHandler::formatRemaining(time_t until) {
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

void StatusHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    String kbd = kbMain();

    // Ensure time is valid
    if (!TimeManager::isValid()) {
        client->sendMessageWithKeyboard(
            evt.chatId,
            "⛔ Time not synced yet. Waiting for NTP…",
            kbd
        );
        return;
    }

    // Pull system snapshot
    SystemStatus s = SystemStatus_get();

    // Battery %
    float v = s.batteryVoltage;
    int pct = (v >= 13.3) ? 100 :
              (v >= 13.1) ?  70 :
              (v >= 12.9) ?  30 :
              (v >= 12.0) ?  10 : 0;

    // Build message
    String m;
    m.reserve(700);

    // ---------------------------------------------------------
    // HEADER
    // ---------------------------------------------------------
    m += "📊 *POULTRY PORTAL v" + String(VERSION) + "*\n";
    m += "━━━━━━━━━━━━━━━\n";

    // ---------------------------------------------------------
    // SYSTEM SUMMARY
    // ---------------------------------------------------------
    m += kv("🧪 Simulation", Config_isSimulatedHardware() ? "ON" : "OFF");
    m += kv("🚪 Door", formatDoorState(s.doorState));
    m += kv("🔋 Battery", String(v, 1) + "V (" + String(pct) + "%)");

    // ENERGY SUMMARY
    m += "⚡ *ENERGY TODAY*\n";
    m += "• System: " + String(EnergySys_getTodaymAh(), 1) + " mAh\n";
    m += "• Motor:  " + String(EnergyMotor_getTodaymAh(), 1) + " mAh\n";

    // TEMP
    m += kv("🌡️ Temp", String(s.temperatureC, 1) + "°C " + formatTempLabel(s.temperatureC));

    // LOCATION
    m += kv("📍 Location",
            String(Config_getLat(), 4) + ", " + String(Config_getLong(), 4));

    // UPTIME
    m += kv("⏱️ Uptime", TimeUtils::getUptime());

    // MODE
    if (remoteOverride) {
        String left = formatRemaining(remoteOverrideUntil);
        String untilStr = TimeUtils::formatTimestamp(remoteOverrideUntil);
        m += kv("⚙️ Mode", "MANUAL 🛠️ (" + left + " left)");
        m += kv("⏳ Until", untilStr);
    } else {
        m += kv("⚙️ Mode", "AUTO 🤖");
    }

    m += "━━━━━━━━━━━━━━━\n";

    // ---------------------------------------------------------
    // SCHEDULER
    // ---------------------------------------------------------
    m += kv("🌅 Next Open", Scheduler_getNextOpen());
    m += kv("🌇 Next Close", Scheduler_getNextClose());

    // ---------------------------------------------------------
    // SEND
    // ---------------------------------------------------------
    client->sendMessageWithKeyboard(evt.chatId, m, kbd);
}
