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

void StatusHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    String kbd = kbMain();

    // Ensure time is valid before showing anything
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

    // ---------------------------------------------------------
    // Battery percentage estimation
    // ---------------------------------------------------------
    float v = s.batteryVoltage;
    int pct = (v >= 13.3) ? 100 :
              (v >= 13.1) ?  70 :
              (v >= 12.9) ?  30 :
              (v >= 12.0) ?  10 : 0;

    // ---------------------------------------------------------
    // Door state
    // ---------------------------------------------------------
    String stateStr;
    switch (s.doorState) {
        case M_OPEN:    stateStr = "OPEN ✅"; break;
        case M_CLOSED:  stateStr = "CLOSED 🌙"; break;
        case M_OPENING: stateStr = "OPENING ⚙️"; break;
        case M_CLOSING: stateStr = "CLOSING ⚙️"; break;
        case M_STUCK:   stateStr = "STUCK ⚠️"; break;
    }

    // ---------------------------------------------------------
    // Temperature label
    // ---------------------------------------------------------
    String tempLabel;
    if (s.temperatureC >= 40) tempLabel = "🔥 HOT";
    else if (s.temperatureC <= 0) tempLabel = "❄️ COLD";
    else tempLabel = "🙂 OK";

    // ---------------------------------------------------------
    // Build message
    // ---------------------------------------------------------
    String m;
    m.reserve(600); // avoid fragmentation

    m += "📊 *POULTRY PORTAL v" + String(VERSION) + "*\n";
    m += "━━━━━━━━━━━━━━━\n";

    m += "🧪 Simulation: ";
    m += Config_isSimulatedHardware() ? "ON\n" : "OFF\n";

    m += "🚪 Door: " + stateStr + "\n";
    m += "🔋 Batt: " + String(v, 1) + "V (" + String(pct) + "%)\n";
    m += "⚡ Today: " + String(Energy_getTodaymAh(), 1) + " mAh\n";
    m += "🌡️ Temp: " + String(s.temperatureC, 1) + "°C " + tempLabel + "\n";

    m += "📍 Location: ";
    m += String(Config_getLat(), 4) + ", " + String(Config_getLong(), 4) + "\n";

    m += "⏱️ Uptime: " + TimeUtils::getUptime() + "\n";

    m += "⚙️ Mode: ";
    m += remoteOverride ? "MANUAL 🛠️\n" : "AUTO 🤖\n";

    m += "━━━━━━━━━━━━━━━\n";

    char buf[16];

    m += "🌅 Next Open:  "  + Scheduler_getNextOpen()  + "\n";
    m += "🌇 Next Close: "  + Scheduler_getNextClose() + "\n";

    // ---------------------------------------------------------
    // Send
    // ---------------------------------------------------------
    client->sendMessageWithKeyboard(evt.chatId, m, kbd);
}