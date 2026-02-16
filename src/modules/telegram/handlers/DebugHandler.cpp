#include "DebugHandler.h"

#include "../../keyboards/TelegramKeyboards.h"
#include "../../config/Config.h"
#include "../../motor/MotorModule.h"
#include "../../motor/MotorTask.h"
#include "../../energy/EnergyModule.h"
#include "../../time/TimeManager.h"
#include "modules/utils/TimeUtils.h"
#include "modules/scheduler/SchedulerTask.h"
#include "modules/scheduler/SunContext.h"
#include "../../motor/MotorPins.h"
#include "../../system/Logging.h"

extern bool debugMenuEnabled;
extern bool remoteOverride;
extern SunSet Sun;  

namespace DebugHandler {

// ---------------------------------------------------------
// Helpers
// ---------------------------------------------------------
static float getMotorCurrent() {
    return Motor_getCurrentmA();
}

static void sendDebugMenu(uint64_t chatId, TelegramClient* client) {
    client->sendMessageWithKeyboard(
        chatId,
        "🛠 *DEBUG MENU*\n━━━━━━━━━━━━━━\nChoose a debug function:",
        kbDebug()
    );
}

// ---------------------------------------------------------
// DEBUG DOOR
// ---------------------------------------------------------
static void debugDoor(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(400);

    float current = getMotorCurrent();

    out += "🛠 *DEBUG DOOR*\n";
    out += "━━━━━━━━━━━━━━\n";

    out += "🚪 State: ";
    switch (Motor_getState()) {
        case M_OPEN:    out += "OPEN"; break;
        case M_CLOSED:  out += "CLOSED"; break;
        case M_OPENING: out += "OPENING"; break;
        case M_CLOSING: out += "CLOSING"; break;
        case M_STUCK:   out += "STUCK"; break;
    }
    out += "\n\n";

    out += "🔌 Motor Current:     " + String(current, 1) + " mA\n";
    out += "⚠️ Stall Threshold:   " + String(Config_getPinchThreshold()) + " mA\n\n";

    out += "📉 *Stall Analysis*\n";

    float pct = (current / Config_getPinchThreshold()) * 100.0;
    out += "   • Threshold Usage: " + String(pct, 1) + "%\n";

    if (pct < 50) out += "   • Status: Safe 🟢\n";
    else if (pct < 90) out += "   • Status: Elevated ⚠️\n";
    else if (pct < 100) out += "   • Status: Near Stall 🔶\n";
    else out += "   • Status: STALL TRIGGER 🚨\n";

    bool wouldStall = (current > Config_getPinchThreshold());
    out += "   • Would Stall Now: " + String(wouldStall ? "YES 🚨" : "No") + "\n\n";

    out += "🔘 Limit Open:        " + String(digitalRead(PIN_LIMIT_OPEN)) + "\n";
    out += "🔘 Limit Close:       " + String(digitalRead(PIN_LIMIT_CLOSE)) + "\n";
    out += "🔘 Switch Open:       " + String(digitalRead(PIN_SWITCH_OPEN)) + "\n";
    out += "🔘 Switch Close:      " + String(digitalRead(PIN_SWITCH_CLOSE)) + "\n\n";

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG TIME
// ---------------------------------------------------------
static void debugTime(uint64_t chatId, TelegramClient* client) {
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    struct tm* utc   = gmtime(&now);

    String out = "🛠 *DEBUG TIME*\n━━━━━━━━━━━━━━\n";

    if (!local) {
        out += "RTC not valid yet\n";
        client->sendMessage(chatId, out);
        return;
    }

    out += "⏱ Epoch: " + String((uint32_t)now) + "\n";

    out += "🕒 Local: ";
    out += String(local->tm_year + 1900) + "-";
    out += String(local->tm_mon + 1) + "-";
    out += String(local->tm_mday) + " ";
    out += String(local->tm_hour) + ":";
    out += String(local->tm_min) + ":";
    out += String(local->tm_sec) + "\n";

    out += "🌍 UTC:   ";
    out += String(utc->tm_year + 1900) + "-";
    out += String(utc->tm_mon + 1) + "-";
    out += String(utc->tm_mday) + " ";
    out += String(utc->tm_hour) + ":";
    out += String(utc->tm_min) + ":";
    out += String(utc->tm_sec) + "\n\n";

    int tzHours = TimeUtils::getUTCOffsetHours();
    out += "⏳ TZ Offset: " + String(tzHours) + "h\n";
    out += "🕰 DST Active: " + String(local->tm_isdst ? "yes" : "no") + "\n\n";

    out += "⏱ Uptime: " + TimeUtils::getUptime() + "\n";

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG SUN
// ---------------------------------------------------------
static void debugSun(uint64_t chatId, TelegramClient* client) {
    if (!TimeUtils::timeIsValid()) {
        client->sendMessage(chatId, "⛔ Time not synced yet. Waiting for NTP...");
        return;
    }

    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);

    if (!ptm) {
        client->sendMessage(chatId, "Time not valid yet");
        return;
    }

    int year  = ptm->tm_year + 1900;
    int month = ptm->tm_mon + 1;
    int day   = ptm->tm_mday;

    int tzHours = TimeUtils::getUTCOffsetHours();

    sun.setPosition(Config_getLat(), Config_getLong(), tzHours);
    sun.setCurrentDate(year, month, day);

    int sunriseLocal, sunsetLocal;
    Scheduler_calcLocalSunTimes(sunriseLocal, sunsetLocal);

    int srH = sunriseLocal / 60;
    int srM = sunriseLocal % 60;
    int ssH = sunsetLocal / 60;
    int ssM = sunsetLocal % 60;

    String out = "🛠 *DEBUG SUN*\n";
    out += "━━━━━━━━━━━━━━\n";
    out += "📍 Lat: " + String(Config_getLat(), 4) + "\n";
    out += "📍 Lon: " + String(Config_getLong(), 4) + "\n";
    out += "⏱ TZ Offset: " + String(tzHours) + "h\n";
    out += "📅 Today: " + String(month) + "/" + String(day) + "\n";

    out += "🌅 Sunrise: "
        + String(srH) + ":" + (srM < 10 ? "0" : "") + String(srM) + "\n";

    out += "🌇 Sunset:  "
        + String(ssH) + ":" + (ssM < 10 ? "0" : "") + String(ssM) + "\n\n";

    out += "🔧 Offsets: open=" + String(Config_getOpenOffset())
        + "  close=" + String(Config_getCloseOffset()) + "\n";

    out += "🧠 Smart Open:  " + Scheduler_getNextOpen() + "\n";
    out += "🧠 Smart Close: " + Scheduler_getNextClose() + "\n";

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG AUTO
// ---------------------------------------------------------
static void debugAuto(uint64_t chatId, TelegramClient* client) {
    String logicState = "";
    switch (Motor_getState()) {
        case M_OPEN:    logicState = "OPEN"; break;
        case M_CLOSED:  logicState = "CLOSED"; break;
        case M_OPENING: logicState = "OPENING"; break;
        case M_CLOSING: logicState = "CLOSING"; break;
        case M_STUCK:   logicState = "STUCK"; break;
    }

    String out = "🤖 *AUTO DEBUG*\n";
    out += "Logic State: " + logicState + "\n";
    out += "remoteOverride: " + String(remoteOverride ? "true" : "false") + "\n";

    client->sendMessageWithKeyboard(chatId, out, kbDebug());
}

// ---------------------------------------------------------
// DEBUG STATE
// ---------------------------------------------------------
static void debugState(uint64_t chatId, TelegramClient* client) {
    String out = "⚙️ *STATE DEBUG*\n";

    switch (Motor_getState()) {
        case M_OPEN:    out += "Door State: OPEN\n"; break;
        case M_CLOSED:  out += "Door State: CLOSED\n"; break;
        case M_OPENING: out += "Door State: OPENING\n"; break;
        case M_CLOSING: out += "Door State: CLOSING\n"; break;
        case M_STUCK:   out += "Door State: STUCK\n"; break;
    }

    out += "remoteOverride: " + String(remoteOverride ? "true" : "false") + "\n";

    client->sendMessageWithKeyboard(chatId, out, kbDebug());
}

// ---------------------------------------------------------
// DEBUG LIMITS
// ---------------------------------------------------------
static void debugLimits(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(300);

    bool openHit  = (digitalRead(PIN_LIMIT_OPEN)  == LOW);
    bool closeHit = (digitalRead(PIN_LIMIT_CLOSE) == LOW);

    out += "🛠 *DEBUG LIMITS*\n";
    out += "━━━━━━━━━━━━━━━\n";
    out += "Limit Open (raw):  " + String(digitalRead(PIN_LIMIT_OPEN)) + "\n";
    out += "Limit Close (raw): " + String(digitalRead(PIN_LIMIT_CLOSE)) + "\n";
    out += "Open HIT:  "  + String(openHit  ? "YES" : "no") + "\n";
    out += "Close HIT: "  + String(closeHit ? "YES" : "no") + "\n";

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG ENERGY
// ---------------------------------------------------------
static void debugEnergy(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(300);

    out += "🛠 *DEBUG ENERGY*\n";
    out += "━━━━━━━━━━━━━━━\n";
    out += "Total Used mAh:    " + String(Energy_getTodaymAh(), 2) + "\n";

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG CONFIG
// ---------------------------------------------------------
static void debugConfig(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(400);

    out += "🛠 *DEBUG CONFIG*\n";
    out += "━━━━━━━━━━━━━━━\n";

    out += "Open Offset:       " + String(Config_getOpenOffset()) + " min\n";
    out += "Close Offset:      " + String(Config_getCloseOffset()) + " min\n";
    out += "Timezone:          " + Config_getTimezone() + "\n";
    out += "Motor Timeout:     " + String(Config_getMotorTimeout()) + " sec\n";
    out += "Pinch Threshold:   " + String(Config_getPinchThreshold()) + " mA\n";
    out += "Debug Menu:        " + String(debugMenuEnabled ? "ENABLED" : "disabled") + "\n";

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG ALL (Full System Snapshot)
// ---------------------------------------------------------
static void debugAll(uint64_t chatId, TelegramClient* client) {

    String out;
    out.reserve(1200);

    out += "📑 *FULL DEBUG*\n";
    out += "━━━━━━━━━━━━━━━━━━\n\n";

    // -----------------------------------------------------
    // DOOR
    // -----------------------------------------------------
    out += "🚪 *DOOR*\n";

    out += "• State: ";
    switch (Motor_getState()) {
        case M_OPEN:    out += "OPEN"; break;
        case M_CLOSED:  out += "CLOSED"; break;
        case M_OPENING: out += "OPENING"; break;
        case M_CLOSING: out += "CLOSING"; break;
        case M_STUCK:   out += "STUCK"; break;
        default:        out += "UNKNOWN"; break;
    }
    out += "\n";

    out += "• Health: " + Motor_getHealthString() + "\n";
    out += "• Override: " + String(remoteOverride ? "true" : "false") + "\n";
    out += "• Open Cycles: " + String(Motor_getOpenCycles()) + "\n";
    out += "• Close Cycles: " + String(Motor_getCloseCycles()) + "\n\n";

    // -----------------------------------------------------
    // MOTOR
    // -----------------------------------------------------
    out += "🔌 *MOTOR*\n";

    float current = Motor_getCurrentmA();
    int pinch     = Config_getPinchThreshold();

    out += "• Current: " + String(current, 1) + " mA\n";
    out += "• Pinch Threshold: " + String(pinch) + " mA\n";
    out += "• Timeout: " + String(Config_getMotorTimeout()) + " sec\n";

    bool wouldStall = current > pinch;
    out += "• Would Stall Now: " + String(wouldStall ? "YES 🚨" : "No") + "\n";

    out += "• Last Command: " + Motor_getLastCommandString() + "\n";
    out += "• Last Motion Start: " + Motor_getLastMotionTimestamp() + "\n\n";

    // -----------------------------------------------------
    // LIMIT SWITCHES
    // -----------------------------------------------------
    out += "🔘 *LIMIT SWITCHES*\n";

    bool openHit  = (digitalRead(PIN_LIMIT_OPEN)  == LOW);
    bool closeHit = (digitalRead(PIN_LIMIT_CLOSE) == LOW);

    out += "• Limit Open:  "  + String(openHit  ? "HIT" : "not hit") + "\n";
    out += "• Limit Close: "  + String(closeHit ? "HIT" : "not hit") + "\n";
    out += "• Switch Open:  " + String(digitalRead(PIN_SWITCH_OPEN)) + "\n";
    out += "• Switch Close: " + String(digitalRead(PIN_SWITCH_CLOSE)) + "\n\n";

    // -----------------------------------------------------
    // SUN / SCHEDULER
    // -----------------------------------------------------
    out += "🌅 *SUN / SCHEDULER*\n";

    int tzHours = TimeUtils::getUTCOffsetHours();

    int sr, ss;
    Scheduler_calcLocalSunTimes(sr, ss);

    int srH = sr / 60, srM = sr % 60;
    int ssH = ss / 60, ssM = ss % 60;

    out += "• Sunrise: " + String(srH) + ":" + (srM < 10 ? "0" : "") + String(srM) + "\n";
    out += "• Sunset:  " + String(ssH) + ":" + (ssM < 10 ? "0" : "") + String(ssM) + "\n";
    out += "• Smart Open:  " + Scheduler_getNextOpen() + "\n";
    out += "• Smart Close: " + Scheduler_getNextClose() + "\n";
    out += "• Open Offset:  " + String(Config_getOpenOffset()) + " min\n";
    out += "• Close Offset: " + String(Config_getCloseOffset()) + " min\n\n";

    // -----------------------------------------------------
    // TIME
    // -----------------------------------------------------
    out += "🕒 *TIME*\n";

    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    struct tm* utc   = gmtime(&now);

    out += "• Epoch: " + String((uint32_t)now) + "\n";

    if (local) {
        out += "• Local: ";
        out += String(local->tm_year + 1900) + "-";
        out += String(local->tm_mon + 1) + "-";
        out += String(local->tm_mday) + " ";
        out += String(local->tm_hour) + ":";
        out += String(local->tm_min) + ":";
        out += String(local->tm_sec) + "\n";
    }

    if (utc) {
        out += "• UTC:   ";
        out += String(utc->tm_year + 1900) + "-";
        out += String(utc->tm_mon + 1) + "-";
        out += String(utc->tm_mday) + " ";
        out += String(utc->tm_hour) + ":";
        out += String(utc->tm_min) + ":";
        out += String(utc->tm_sec) + "\n";
    }

    out += "• TZ Offset: " + String(tzHours) + "h\n";
    out += "• DST: " + String(local && local->tm_isdst ? "yes" : "no") + "\n";
    out += "• Uptime: " + TimeUtils::getUptime() + "\n\n";

    // -----------------------------------------------------
    // SYSTEM
    // -----------------------------------------------------
    out += "📡 *SYSTEM*\n";

    out += "• Simulation Mode: " + String(Config_isSimulatedHardware() ? "ON 🧪" : "OFF") + "\n";
    out += "• Debug Menu: " + String(debugMenuEnabled ? "ENABLED" : "disabled") + "\n";
    out += "• Chat ID: " + String(chatId) + "\n\n";

    // -----------------------------------------------------
    // ENERGY
    // -----------------------------------------------------
    out += "⚡ *ENERGY*\n";
    out += "• Today Used: " + String(Energy_getTodaymAh(), 2) + " mAh\n";
    out += "• Peak Current: " + String(Motor_getPeakCurrentmA()) + " mA\n";
    out += "• Avg Current: " + String(Motor_getAverageCurrentmA()) + " mA\n\n";

    // -----------------------------------------------------
    // LAST ACTION
    // -----------------------------------------------------
    out += "📝 *LAST ACTION*\n";
    out += "• " + Log_getLastAction() + "\n";

    client->sendMessageWithKeyboard(chatId, out, kbDebug());
}

// ---------------------------------------------------------
// MAIN HANDLER
// ---------------------------------------------------------
void handle(const TelegramEvent& evt, TelegramClient* client) {

    switch (evt.type) {

        case EVT_SHOW_DEBUG:
            sendDebugMenu(evt.chatId, client);
            return;

        case EVT_DEBUG_DOOR:
            debugDoor(evt.chatId, client);
            return;

        case EVT_DEBUG_TIME:
            debugTime(evt.chatId, client);
            return;

        case EVT_DEBUG_SUN:
            debugSun(evt.chatId, client);
            return;

        case EVT_DEBUG_AUTO:
            debugAuto(evt.chatId, client);
            return;

        case EVT_DEBUG_STATE:
            debugState(evt.chatId, client);
            return;

        case EVT_DEBUG_LIMITS:
            debugLimits(evt.chatId, client);
            return;

        case EVT_DEBUG_ENERGY:
            debugEnergy(evt.chatId, client);
            return;

        case EVT_DEBUG_CONFIG:
            debugConfig(evt.chatId, client);
            return;

        case EVT_DEBUG_ALL:
            debugAll(evt.chatId, client);
            return;

        // -----------------------------------------------------
        // Debug ON/OFF
        // -----------------------------------------------------
        case EVT_DEBUG_ON:
            debugMenuEnabled = true;
            client->sendMessageWithKeyboard(
                evt.chatId,
                "🛠 Debug menu *ENABLED*. You will now see the Debug button in Settings.",
                kbSettings()
            );
            return;

        case EVT_DEBUG_OFF:
            debugMenuEnabled = false;
            client->sendMessageWithKeyboard(
                evt.chatId,
                "🛠 Debug menu *DISABLED*.",
                kbSettings()
            );
            return;

        // -----------------------------------------------------
        // BACK → return to Settings
        // -----------------------------------------------------
        case EVT_BACK:
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⚙️ *SETTINGS*\n━━━━━━━━━━━━━━━\nAdjust system configuration below.",
                kbSettings()
            );
            return;

        default:
            client->sendMessageWithKeyboard(
                evt.chatId,
                "❓ Unknown debug command.",
                kbDebug()
            );
            return;
    }
}

} // namespace DebugHandler