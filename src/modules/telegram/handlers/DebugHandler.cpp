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
#include "modules/battery/BatteryModule.h"

extern bool debugMenuEnabled;
extern bool remoteOverride;
extern SunSet Sun;

// ---------------------------------------------------------
// Formatting Helpers
// ---------------------------------------------------------

String DebugHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

String DebugHandler::kv(const String& label, const String& value) {
    return "• " + label + ": " + value + "\n";
}

String DebugHandler::formatDoorState(int state) {
    switch (state) {
        case M_OPEN:    return "OPEN";
        case M_CLOSED:  return "CLOSED";
        case M_OPENING: return "OPENING";
        case M_CLOSING: return "CLOSING";
        case M_STUCK:   return "STUCK";
        default:        return "UNKNOWN";
    }
}

String DebugHandler::formatBool(bool v, const String& yes, const String& no) {
    return v ? yes : no;
}

// ---------------------------------------------------------
// Debug Menu
// ---------------------------------------------------------

void DebugHandler::sendDebugMenu(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(200);

    out += blockHeader("🛠", "DEBUG MENU");
    out += "Choose a debug function:";

    client->sendMessageWithKeyboard(chatId, out, kbDebug());
}

// ---------------------------------------------------------
// DEBUG DOOR
// ---------------------------------------------------------

void DebugHandler::debugDoor(uint64_t chatId, TelegramClient* client) {
    float current = Motor_getCurrentmA();
    int pinch = Config_getPinchThreshold();

    String out;
    out.reserve(500);

    out += blockHeader("🛠", "DEBUG DOOR");
    out += kv("State", formatDoorState(Motor_getState()));
    out += kv("Motor Current", String(current, 1) + " mA");
    out += kv("Stall Threshold", String(pinch) + " mA");

    float pct = (current / pinch) * 100.0f;
    out += kv("Threshold Usage", String(pct, 1) + "%");

    String risk =
        (pct < 50)  ? "Low" :
        (pct < 90)  ? "Elevated" :
        (pct < 100) ? "Near Stall" :
                      "STALL 🚨";

    out += kv("Stall Risk", risk);
    out += kv("Would Stall Now", formatBool(current > pinch, "YES 🚨", "No"));

    out += kv("Limit Open",  digitalRead(PIN_LIMIT_OPEN)  == LOW ? "HIT" : "not hit");
    out += kv("Limit Close", digitalRead(PIN_LIMIT_CLOSE) == LOW ? "HIT" : "not hit");
    out += kv("Switch Open",  String(digitalRead(PIN_SWITCH_OPEN)));
    out += kv("Switch Close", String(digitalRead(PIN_SWITCH_CLOSE)));

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG TIME
// ---------------------------------------------------------

void DebugHandler::debugTime(uint64_t chatId, TelegramClient* client) {
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    struct tm* utc   = gmtime(&now);

    String out;
    out.reserve(400);

    out += blockHeader("🛠", "DEBUG TIME");

    if (!local) {
        out += "RTC not valid yet";
        client->sendMessage(chatId, out);
        return;
    }

    out += kv("Epoch", String((uint32_t)now));

    char buf[32];

    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
             local->tm_hour, local->tm_min, local->tm_sec);
    out += kv("Local", String(buf));

    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             utc->tm_year + 1900, utc->tm_mon + 1, utc->tm_mday,
             utc->tm_hour, utc->tm_min, utc->tm_sec);
    out += kv("UTC", String(buf));

    out += kv("TZ Offset", String(TimeUtils::getUTCOffsetHours()) + "h");
    out += kv("DST Active", formatBool(local->tm_isdst, "yes", "no"));
    out += kv("Uptime", TimeUtils::getUptime());

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG SUN
// ---------------------------------------------------------

void DebugHandler::debugSun(uint64_t chatId, TelegramClient* client) {
    if (!TimeUtils::timeIsValid()) {
        client->sendMessage(chatId, "⛔ Time not synced yet. Waiting for NTP…");
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

    int sr, ss;
    Scheduler_calcLocalSunTimes(sr, ss);

    int srH = sr / 60, srM = sr % 60;
    int ssH = ss / 60, ssM = ss % 60;

    String out;
    out.reserve(500);

    out += blockHeader("🛠", "DEBUG SUN");
    out += kv("Lat", String(Config_getLat(), 4));
    out += kv("Lon", String(Config_getLong(), 4));
    out += kv("TZ Offset", String(tzHours) + "h");
    out += kv("Today", String(month) + "/" + String(day));

    out += kv("Sunrise", String(srH) + ":" + (srM < 10 ? "0" : "") + String(srM));
    out += kv("Sunset",  String(ssH) + ":" + (ssM < 10 ? "0" : "") + String(ssM));

    out += kv("Open Offset",  String(Config_getOpenOffset()) + " min");
    out += kv("Close Offset", String(Config_getCloseOffset()) + " min");

    out += kv("Smart Open",  Scheduler_getNextOpen());
    out += kv("Smart Close", Scheduler_getNextClose());

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG AUTO
// ---------------------------------------------------------

void DebugHandler::debugAuto(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(200);

    out += blockHeader("🤖", "AUTO DEBUG");
    out += kv("Logic State", formatDoorState(Motor_getState()));
    out += kv("Override", formatBool(remoteOverride));

    client->sendMessageWithKeyboard(chatId, out, kbDebug());
}

// ---------------------------------------------------------
// DEBUG STATE
// ---------------------------------------------------------

void DebugHandler::debugState(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(200);

    out += blockHeader("⚙️", "STATE DEBUG");
    out += kv("Door State", formatDoorState(Motor_getState()));
    out += kv("Override", formatBool(remoteOverride));

    client->sendMessageWithKeyboard(chatId, out, kbDebug());
}

// ---------------------------------------------------------
// DEBUG LIMITS
// ---------------------------------------------------------

void DebugHandler::debugLimits(uint64_t chatId, TelegramClient* client) {
    bool openHit  = (digitalRead(PIN_LIMIT_OPEN)  == LOW);
    bool closeHit = (digitalRead(PIN_LIMIT_CLOSE) == LOW);

    String out;
    out.reserve(300);

    out += blockHeader("🛠", "DEBUG LIMITS");
    out += kv("Limit Open",  openHit  ? "HIT" : "not hit");
    out += kv("Limit Close", closeHit ? "HIT" : "not hit");
    out += kv("Switch Open",  String(digitalRead(PIN_SWITCH_OPEN)));
    out += kv("Switch Close", String(digitalRead(PIN_SWITCH_CLOSE)));

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG ENERGY
// ---------------------------------------------------------

void DebugHandler::debugEnergy(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(600);

    out += blockHeader("⚡", "SYSTEM ENERGY");
    out += kv("Today", String(EnergySys_getTodaymAh(), 2) + " mAh");
    out += kv("Avg Daily (30d)", String(EnergySys_getAvgDailymAh(), 2) + " mAh/day");
    out += kv("30-Day Total", String(EnergySys_getMonthlymAh(), 2) + " mAh");
    out += kv("Peak Current", String(EnergySys_getPeakCurrentmA()) + " mA");
    out += kv("Avg Current", String(EnergySys_getAvgCurrentmA()) + " mA");
    out += kv("Last Reset", EnergySys_getLastResetStr());
    out += "\n";

    out += blockHeader("🔌", "MOTOR ENERGY");
    out += kv("Today", String(EnergyMotor_getTodaymAh(), 2) + " mAh");
    out += kv("Avg Daily (30d)", String(EnergyMotor_getAvgDailymAh(), 2) + " mAh/day");
    out += kv("30-Day Total", String(EnergyMotor_getMonthlymAh(), 2) + " mAh");
    out += kv("Peak Motor Current", String(EnergyMotor_getPeakCurrentmA()) + " mA");
    out += kv("Avg Motor Current", String(EnergyMotor_getAvgCurrentmA()) + " mA");
    out += kv("Last Reset", EnergyMotor_getLastResetStr());

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// DEBUG CONFIG
// ---------------------------------------------------------

void DebugHandler::debugConfig(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(400);

    out += blockHeader("🛠", "DEBUG CONFIG");
    out += kv("Open Offset",  String(Config_getOpenOffset()) + " min");
    out += kv("Close Offset", String(Config_getCloseOffset()) + " min");
    out += kv("Timezone", Config_getTimezone());
    out += kv("Motor Timeout", String(Config_getMotorTimeout()) + " sec");
    out += kv("Pinch Threshold", String(Config_getPinchThreshold()) + " mA");
    out += kv("Debug Menu", debugMenuEnabled ? "ENABLED" : "disabled");

    client->sendMessage(chatId, out);
}

// ---------------------------------------------------------
// FULL DEBUG
// ---------------------------------------------------------

void DebugHandler::debugAll(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(2500);

    out += blockHeader("📑", "FULL DEBUG");
    out += "\n";

    // DOOR
    out += blockHeader("🚪", "DOOR");
    out += kv("State", formatDoorState(Motor_getState()));
    out += kv("Health", Motor_getHealthString());
    out += kv("Override", formatBool(remoteOverride));
    out += kv("Open Cycles", String(Motor_getOpenCycles()));
    out += kv("Close Cycles", String(Motor_getCloseCycles()));
    out += "\n";

    // MOTOR
    float current = Motor_getCurrentmA();
    int pinch = Config_getPinchThreshold();

    out += blockHeader("🔌", "MOTOR");
    out += kv("Current", String(current, 1) + " mA");
    out += kv("Pinch Threshold", String(pinch) + " mA");
    out += kv("Timeout", String(Config_getMotorTimeout()) + " sec");
    out += kv("Would Stall Now", formatBool(current > pinch, "YES 🚨", "No"));
    out += kv("Last Command", Motor_getLastCommandString());
    out += kv("Last Motion Start", Motor_getLastMotionTimestamp());

    float avg = Motor_getAverageTravelTime();
    out += kv("Avg Travel Time",
              String(avg, 1) + " sec (" +
              String(Motor_getTravelSampleCount()) + " samples)");

    time_t st = Motor_getLastStallTime();
    if (st > 0) {
        String ts = TimeUtils::formatTimestamp(st);
        MotorDoorState dir = Motor_getLastStallDirection();
        String d = (dir == M_OPENING) ? "opening" :
                   (dir == M_CLOSING) ? "closing" : "unknown";
        out += kv("Last Stall", ts + " (" + d + ")");
    }

    float duty = Motor_getDutyCycle24h();
    uint32_t ms = Motor_getRuntimeMs24h();
    out += kv("Duty Cycle (24h)", String(duty, 2) + "% (" + String(ms / 1000) + " sec)");
    out += "\n";

    // LIMIT SWITCHES
    out += blockHeader("🔘", "LIMIT SWITCHES");
    out += kv("Limit Open",  digitalRead(PIN_LIMIT_OPEN)  == LOW ? "HIT" : "not hit");
    out += kv("Limit Close", digitalRead(PIN_LIMIT_CLOSE) == LOW ? "HIT" : "not hit");
    out += kv("Switch Open",  String(digitalRead(PIN_SWITCH_OPEN)));
    out += kv("Switch Close", String(digitalRead(PIN_SWITCH_CLOSE)));
    out += "\n";

    // SUN / SCHEDULER
    int tzHours = TimeUtils::getUTCOffsetHours();
    int sr, ss;
    Scheduler_calcLocalSunTimes(sr, ss);

    int srH = sr / 60, srM = sr % 60;
    int ssH = ss / 60, ssM = ss % 60;

    out += blockHeader("🌅", "SUN / SCHEDULER");
    out += kv("Sunrise", String(srH) + ":" + (srM < 10 ? "0" : "") + String(srM));
    out += kv("Sunset",  String(ssH) + ":" + (ssM < 10 ? "0" : "") + String(ssM));
    out += kv("Smart Open", Scheduler_getNextOpen());
    out += kv("Smart Close", Scheduler_getNextClose());
    out += kv("Open Offset",  String(Config_getOpenOffset()) + " min");
    out += kv("Close Offset", String(Config_getCloseOffset()) + " min");
    out += "\n";

    // TIME
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    struct tm* utc   = gmtime(&now);

    out += blockHeader("🕒", "TIME");
    out += kv("Epoch", String((uint32_t)now));

    char buf[32];

    if (local) {
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                 local->tm_hour, local->tm_min, local->tm_sec);
        out += kv("Local", String(buf));
    }

    if (utc) {
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 utc->tm_year + 1900, utc->tm_mon + 1, utc->tm_mday,
                 utc->tm_hour, utc->tm_min, utc->tm_sec);
        out += kv("UTC", String(buf));
    }

    out += kv("TZ Offset", String(tzHours) + "h");
    out += kv("DST", formatBool(local && local->tm_isdst, "yes", "no"));
    out += kv("Uptime", TimeUtils::getUptime());
    out += "\n";

    // SYSTEM
    out += blockHeader("📡", "SYSTEM");
    out += kv("Simulation Mode", Config_isSimulatedHardware() ? "ON 🧪" : "OFF");
    out += kv("Debug Menu", debugMenuEnabled ? "ENABLED" : "disabled");
    out += kv("Chat ID", String(chatId));
    out += "\n";

    // -----------------------------------------------------
    // ENERGY
    // -----------------------------------------------------
    out += blockHeader("⚡", "SYSTEM ENERGY");
    out += kv("Today", String(EnergySys_getTodaymAh(), 2) + " mAh");
    out += kv("Avg Daily (30d)", String(EnergySys_getAvgDailymAh(), 2) + " mAh/day");
    out += kv("30-Day Total", String(EnergySys_getMonthlymAh(), 2) + " mAh");
    out += kv("Peak Current", String(EnergySys_getPeakCurrentmA()) + " mA");
    out += kv("Avg Current", String(EnergySys_getAvgCurrentmA()) + " mA");
    out += kv("Last Reset", EnergySys_getLastResetStr());
    out += "\n";

    out += blockHeader("🔌", "MOTOR ENERGY");
    out += kv("Today", String(EnergyMotor_getTodaymAh(), 2) + " mAh");
    out += kv("Avg Daily (30d)", String(EnergyMotor_getAvgDailymAh(), 2) + " mAh/day");
    out += kv("30-Day Total", String(EnergyMotor_getMonthlymAh(), 2) + " mAh");
    out += kv("Peak Motor Current", String(EnergyMotor_getPeakCurrentmA()) + " mA");
    out += kv("Avg Motor Current", String(EnergyMotor_getAvgCurrentmA()) + " mA");
    out += kv("Last Reset", EnergyMotor_getLastResetStr());
    out += "\n";

    // -----------------------------------------------------
    // BATTERY
    // -----------------------------------------------------
    float minV = Battery_getMinToday();
    float maxV = Battery_getMaxToday();

    out += blockHeader("🔋", "BATTERY");
    out += kv("Today", String(maxV, 2) + "V max / " + String(minV, 2) + "V min");
    out += "\n";

    // -----------------------------------------------------
    // LAST ACTION
    // -----------------------------------------------------
    out += blockHeader("📝", "LAST ACTION");
    out += "• " + Log_getLastAction() + "\n";

    // -----------------------------------------------------
    // SEND
    // -----------------------------------------------------
    client->sendMessageWithKeyboard(chatId, out, kbDebug());
}

// ---------------------------------------------------------
// MAIN HANDLER
// ---------------------------------------------------------
void DebugHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

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
