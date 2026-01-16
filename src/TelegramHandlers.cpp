#include "TelegramHandlers.h"
#include "Globals.h"
#include "Config.h"
#include "Scheduler.h"
#include "Motor.h"
#include "Energy.h"
#include "Battery.h"
#include "TimeUtils.h"
#include "Display.h"
#include "SystemStatus.h"
#include "Temperature.h"
#include "Logging.h"
#include "TimeUtils.h"

#include <AsyncTelegram2.h>
#include <WiFi.h>
#include <time.h>

extern AsyncTelegram2 bot;

// ---------------------------------------------------------
//  KEYBOARD BUILDERS (declared in TelegramRouter.cpp)
// ---------------------------------------------------------
extern ReplyKeyboard buildMainKeyboard();
extern ReplyKeyboard buildSettingsKeyboard();
extern ReplyKeyboard buildDebugKeyboard();

// ---------------------------------------------------------
//  STATUS
// ---------------------------------------------------------
void handleStatus(TBMessage &msg) {
    menuState = MENU_MAIN;
    ReplyKeyboard kbd = buildMainKeyboard();

    if (!TimeUtils_timeIsValid()) {
        bot.sendMessage(msg, "⛔ Time not synced yet. Waiting for NTP...");
        return;
    }

    float v = Battery_getVoltage();
    int pct = (v >= 13.3) ? 100 :
              (v >= 13.1) ? 70 :
              (v >= 12.9) ? 30 :
              (v >= 12.0) ? 10 : 0;

    String stateStr;
    switch (Motor_getState()) {
        case M_OPEN:    stateStr = "OPEN ✅"; break;
        case M_CLOSED:  stateStr = "CLOSED 🌙"; break;
        case M_OPENING: stateStr = "OPENING ⚙️"; break;
        case M_CLOSING: stateStr = "CLOSING ⚙️"; break;
        case M_STUCK:   stateStr = "STUCK ⚠️"; break;
    }

    int sunriseLocal, sunsetLocal;
    Scheduler_calcLocalSunTimes(sunriseLocal, sunsetLocal);

    int srH = sunriseLocal / 60;
    int srM = sunriseLocal % 60;
    int ssH = sunsetLocal / 60;
    int ssM = sunsetLocal % 60;

    String m = "📊 *POULTRY PORTAL v" + String(VERSION) + "*\n━━━━━━━━━━━━━━━\n";
    m += "🚪 Door: " + stateStr + "\n";
    m += "🔋 Batt: " + String(v, 1) + "V (" + String(pct) + "%)\n";
    m += "⚡ Today: " + String(Energy_getTodaymAh(), 1) + " mAh\n";
    m += "🌡️ Temp: " + String(Temperature_getCelsius(), 1) + "°C " + Temperature_getLabel() + "\n";
    m += "📍 Location: " + String(Config_getLat(), 4) + ", " + String(Config_getLong(), 4) + "\n";
    m += "⏱️ Uptime: " + TimeUtils_getUptime() + "\n";
    m += "⚙️ Mode: " + String(remoteOverride ? "MANUAL 🛠️" : "AUTO 🤖") + "\n";
    m += "━━━━━━━━━━━━━━━\n";

    m += "🌅 Next Open:  " + String(srH) + ":" + (srM < 10 ? "0" : "") + String(srM) +
         " (+" + String(Config_getOpenOffset()) + ")\n";

    m += "🌇 Next Close: " + String(ssH) + ":" + (ssM < 10 ? "0" : "") + String(ssM) +
         " (+" + String(Config_getCloseOffset()) + ")\n";

    bot.sendMessage(msg, m.c_str(), kbd);
}

// ---------------------------------------------------------
//  ENERGY
// ---------------------------------------------------------
void handleEnergy(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();

    String dp = "";
    for (int i = 29; i >= 0; i--) {
        dp += String(Energy_getMonthlymAh() / 30.0, 0);
        if (i > 0) dp += ",";
    }

    String avg = dp;

    String json = "{"
      "type:'line',"
      "data:{"
        "labels:[30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1],"
        "datasets:["
          "{label:'Daily mAh',data:[" + dp + "],fill:true,"
          "backgroundColor:'rgba(54,162,235,0.2)',"
          "borderColor:'rgb(54,162,235)',"
          "borderWidth:2},"
          "{label:'7-Day Avg',data:[" + avg + "],fill:false,"
          "borderColor:'rgb(255,99,132)',"
          "borderWidth:2,"
          "tension:0.3}"
        "]"
      "},"
      "options:{"
        "plugins:{"
          "title:{display:true,text:'30-Day Energy Usage',font:{size:18}}"
        "},"
        "scales:{"
          "x:{grid:{display:true}},"
          "y:{grid:{display:true}}"
        "}"
      "}"
    "}";

    json.replace(" ", "");

    String chartPng = "https://quickchart.io/chart.png?chart=" + json;

    bot.sendPhoto(msg, chartPng.c_str(), "");
}

// ---------------------------------------------------------
//  LOGS
// ---------------------------------------------------------
void handleLogs(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();

    int count = getLogCount();
    if (count == 0) {
        String out = "📋 *RECENT ACTIVITY*\n━━━━━━━━━━━━━━━\nNo logs yet.";
        bot.sendMessage(msg, out.c_str(), kbd);
        return;
    }

    String logMsg = "📋 *RECENT ACTIVITY*\n━━━━━━━━━━━━━━━\n";
    for (int i = 0; i < count; i++) {
        LogEntry e = getLog(i);
        if (e.message.length() == 0) continue;

        struct tm *tm_info = localtime(&e.timestamp);
        if (tm_info) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d-%02d %02d:%02d",
                     tm_info->tm_mon + 1,
                     tm_info->tm_mday,
                     tm_info->tm_hour,
                     tm_info->tm_min);
            logMsg += "[";
            logMsg += buf;
            logMsg += "] ";
        }
        logMsg += e.message;
        logMsg += "\n";
    }

    bot.sendMessage(msg, logMsg.c_str(), kbd);
}

// ---------------------------------------------------------
//  OPEN
// ---------------------------------------------------------
void handleOpen(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();
    remoteOverride = true;

    Motor_requestOpen();
    addLog("Remote Open 📱");
    bot.sendMessage(msg, "🔓 Opening door...", kbd);
}

// ---------------------------------------------------------
//  CLOSE
// ---------------------------------------------------------
void handleClose(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();
    remoteOverride = true;

    if (digitalRead(PIN_LIMIT_CLOSE) == LOW) {
        bot.sendMessage(msg, "🌙 Door is already CLOSED", kbd);
        return;
    }

    Motor_requestClose();
    addLog("Remote Close 📱");
    bot.sendMessage(msg, "🔒 Closing door...", kbd);
}

// ---------------------------------------------------------
//  AUTO
// ---------------------------------------------------------
void handleAuto(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();
    remoteOverride = false;

    if (!TimeUtils_timeIsValid()) return;

    if (Motor_getState() == M_STUCK) {
        Motor_stop();
        addLog("Auto Mode → cleared STUCK");
    }

    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);

    if (!ptm) {
        bot.sendMessage(msg,
            "🤖 Auto Mode Enabled\n"
            "⏳ Time not available yet.",
            kbd
        );
        addLog("Auto Mode → time unavailable");
        return;
    }

    int sunriseLocal, sunsetLocal;
    Scheduler_calcLocalSunTimes(sunriseLocal, sunsetLocal);

    int minutesNow = ptm->tm_hour * 60 + ptm->tm_min;
    bool shouldBeOpen = (minutesNow >= sunriseLocal && minutesNow < sunsetLocal);

    if (Battery_getVoltage() < CRITICAL_VOLTAGE) {
        bot.sendMessage(msg,
            "🪫 Auto Mode Enabled\n"
            "⚠️ Cannot move door: critical battery.",
            kbd
        );
        addLog("Auto Mode → blocked (critical battery)");
        return;
    }

    if (Temperature_isCritical()) {
        bot.sendMessage(msg,
            "🔥 Auto Mode Enabled\n"
            "⚠️ Cannot move door: critical temperature.",
            kbd
        );
        addLog("Auto Mode → blocked (critical temperature)");
        return;
    }

    if (shouldBeOpen && Motor_getState() != M_OPEN) {
        Motor_requestOpen();
        addLog("Auto Mode → Correcting to OPEN");
        bot.sendMessage(msg, "🔄 Auto Mode: Opening door to match schedule", kbd);
    }
    else if (!shouldBeOpen && Motor_getState() != M_CLOSED) {
        Motor_requestClose();
        addLog("Auto Mode → Correcting to CLOSED");
        bot.sendMessage(msg, "🔄 Auto Mode: Closing door to match schedule", kbd);
    }
    else {
        bot.sendMessage(msg,
            "🤖 Auto Mode Enabled\n"
            "Door is already in the correct position.",
            kbd
        );
    }

    addLog("Auto Mode 🤖");
}

// ---------------------------------------------------------
//  HEALTH
// ---------------------------------------------------------
void handleHealth(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();

    size_t freeHeap = ESP.getFreeHeap();
    size_t minHeap  = ESP.getMinFreeHeap();
    float vBatt     = Battery_getVoltage();
    float motorA    = Motor_getCurrent();
    int rssi        = WiFi.RSSI();
    float avgDaily  = Energy_getAvgDailymAh();
    float monthly   = Energy_getMonthlymAh();
    String uptime   = TimeUtils_getUptime();
    esp_reset_reason_t reason = esp_reset_reason();

    String out = "🩺 *SYSTEM HEALTH*\n━━━━━━━━━━━━━━━\n";

    out += "📶 WiFi RSSI: " + String(rssi) + " dBm\n";
    out += "🔋 Battery: " + String(vBatt, 2) + "V\n";
    out += "⚡ Motor Current: " + String(motorA, 0) + " mA\n";
    out += "🌡️ ESP Temp: " + String(Temperature_getCelsius(), 1) + "°C " + Temperature_getLabel() + "\n";
    out += "⏱️ Uptime: " + uptime + "\n";
    out += "📈 Avg Daily: " + String(avgDaily, 1) + " mAh/day\n";
    out += "📅 Last 30 Days: " + String(monthly, 1) + " mAh\n";
    out += "🔁 Last Reset Reason: " + String((int)reason) + "\n";
    out += "💾 Free Heap: " + String(freeHeap) + " bytes\n";
    out += "📉 Min Heap: " + String(minHeap) + " bytes\n";
    out += "🧠 Health: " + SystemStatus_getHealthLabel() + "\n\n";

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

    bot.sendMessage(msg, out.c_str(), kbd);
}

// ---------------------------------------------------------
//  SETTINGS MENU
// ---------------------------------------------------------
void handleSettings(TBMessage &msg) {
    menuState = MENU_SETTINGS;

    ReplyKeyboard kbd = buildSettingsKeyboard();

    String text =
        "⚙️ SETTINGS\n"
        "━━━━━━━━━━━━━━\n"
        "Choose a setting to adjust:";

    bot.sendMessage(msg, text.c_str(), kbd);
}

// ---------------------------------------------------------
//  TIMEZONE
// ---------------------------------------------------------
void handleTimezone(TBMessage &msg) {
    menuState = MENU_TIMEZONE;

    ReplyKeyboard kbd;
    kbd.addButton("Central");
    kbd.addButton("Eastern");
    kbd.addButton("Mountain");
    kbd.addButton("Pacific");
    kbd.addButton("Back");

    String val = msg.text;
    val.trim();
    val.toLowerCase();

    if (val == "back" || val == "/back") {
        handleSettings(msg);
        return;
    }

    if (val == "central")
        Config_setTimezone("CST6CDT,M3.2.0,M11.1.0");
    else if (val == "eastern")
        Config_setTimezone("EST5EDT,M3.2.0,M11.1.0");
    else if (val == "mountain")
        Config_setTimezone("MST7MDT,M3.2.0,M11.1.0");
    else if (val == "pacific")
        Config_setTimezone("PST8PDT,M3.2.0,M11.1.0");
    else {
        bot.sendMessage(msg, "Choose a timezone:", kbd);
        return;
    }

    Config_save();

    String out = "🌐 Timezone updated to " + val;
    bot.sendMessage(msg, out.c_str(), kbd);
}

// ---------------------------------------------------------
//  LOCATION
// ---------------------------------------------------------
void handleLocation(TBMessage &msg) {
    menuState = MENU_SETTINGS;
    ReplyKeyboard kbd = buildSettingsKeyboard();

    String text = msg.text;
    text.trim();
    String lower = text;
    lower.toLowerCase();

    if (lower == "back") {
        handleSettings(msg);
        return;
    }

    if (msg.location.latitude != 0.0 &&
        msg.location.longitude != 0.0 &&
        abs(msg.location.latitude)  > 0.1 &&
        abs(msg.location.longitude) > 0.1) {

        Config_setLat(msg.location.latitude);
        Config_setLong(msg.location.longitude);
        Config_save();

        String out = "📍 Location updated from Telegram pin:\n";
        out += "Lat: " + String(Config_getLat(), 4) + "\n";
        out += "Lon: " + String(Config_getLong(), 4);

        bot.sendMessage(msg, out.c_str(), kbd);
        return;
    }

    if (lower == "location") {
        bot.sendMessage(
            msg,
            "Send your location in one of these ways:\n"
            "• Drop a Telegram location pin\n"
            "• Or type:  /location 30.3011,-97.3301",
            kbd
        );
        return;
    }

    if (lower.startsWith("/location")) {
        String args = text.substring(9);
        args.trim();

        if (args.length() == 0) {
            bot.sendMessage(
                msg,
                "Usage:\n/location 30.3011,-97.3301",
                kbd
            );
            return;
        }

        float lat, lon;
        if (sscanf(args.c_str(), "%f,%f", &lat, &lon) != 2) {
            bot.sendMessage(
                msg,
                "❌ Invalid format.\nUse: /location 30.3011,-97.3301",
                kbd
            );
            return;
        }

        if (lat < -90.0f || lat > 90.0f ||
            lon < -180.0f || lon > 180.0f) {

            bot.sendMessage(
                msg,
                "❌ Invalid coordinates.\n"
                "Latitude must be -90 to 90.\n"
                "Longitude must be -180 to 180.",
                kbd
            );
            return;
        }

        Config_setLat(lat);
        Config_setLong(lon);
        Config_save();

        String out = "📍 Location updated:\n";
        out += "Lat: " + String(Config_getLat(), 4) + "\n";
        out += "Lon: " + String(Config_getLong(), 4);

        bot.sendMessage(msg, out.c_str(), kbd);
        return;
    }

    bot.sendMessage(
        msg,
        "❌ Unknown location command.\n"
        "Send a Telegram location pin or type:\n"
        "/location 30.3011,-97.3301",
        kbd
    );
}

// ---------------------------------------------------------
//  OFFSETS
// ---------------------------------------------------------
void handleOffsets(TBMessage &msg) {
    menuState = MENU_SETTINGS;

    ReplyKeyboard kbd = buildSettingsKeyboard();

    String text = msg.text;
    text.trim();
    String lower = text;
    lower.toLowerCase();

    if (lower == "back") {
        waitingForOpenOffset = false;
        waitingForCloseOffset = false;
        handleSettings(msg);
        return;
    }

    if (!waitingForOpenOffset && !waitingForCloseOffset &&
        lower == "openoffset") {

        waitingForOpenOffset = true;
        waitingForCloseOffset = false;

        bot.sendMessage(
            msg,
            "🌅 Set Open Offset\n\n"
            "Type a number between -180 and +180 minutes.",
            kbd
        );
        return;
    }

    if (!waitingForOpenOffset && !waitingForCloseOffset &&
        lower == "closeoffset") {

        waitingForOpenOffset = false;
        waitingForCloseOffset = true;

        bot.sendMessage(
            msg,
            "🌇 Set Close Offset\n\n"
            "Type a number between -180 and +180 minutes.",
            kbd
        );
        return;
    }

    if (waitingForOpenOffset || waitingForCloseOffset) {

        if (!StringUtils_isNumber(lower)) {
            bot.sendMessage(msg, "⛔ Please enter a valid number.", kbd);
            return;
        }

        int val = lower.toInt();
        if (val < -180 || val > 180) {
            bot.sendMessage(msg, "⛔ Value must be between -180 and +180.", kbd);
            return;
        }

        if (waitingForOpenOffset) {
            Config_setOpenOffset(val);
            Config_save();
        }

        if (waitingForCloseOffset) {
            Config_setCloseOffset(val);
            Config_save();
        }

        waitingForOpenOffset = false;
        waitingForCloseOffset = false;

        String out = "✔ Offset updated.\n";
        out += "🌅 Open: " + String(Config_getOpenOffset()) + " min\n";
        out += "🌇 Close: " + String(Config_getCloseOffset()) + " min";

        bot.sendMessage(msg, out.c_str(), kbd);
        return;
    }

    bot.sendMessage(msg, "❓ Unknown command.\nUse OpenOffset or CloseOffset.", kbd);
}

// ---------------------------------------------------------
//  PINCH THRESHOLD
// ---------------------------------------------------------
void handlePinchThreshold(TBMessage &msg) {
    menuState = MENU_SETTINGS;

    ReplyKeyboard kbd = buildSettingsKeyboard();

    String text = msg.text;
    text.trim();
    String lower = text;
    lower.toLowerCase();

    if (lower == "back" || lower == "/back") {
        handleSettings(msg);
        return;
    }

    if (lower == "pinch threshold" || lower == "/setpinch") {
        bot.sendMessage(
            msg,
            "🧲 Pinch Threshold (Stall Current)\n\n"
            "Type a value between 200 and 2000 mA.\n"
            "Example: 900",
            kbd
        );
        return;
    }

    char *end;
    long val = strtol(lower.c_str(), &end, 10);

    if (*end == '\0') {
        if (val < 200 || val > 2000) {
            bot.sendMessage(msg, "⛔ Value must be between 200 and 2000 mA.", kbd);
            return;
        }

        Config_setPinchThreshold(val);
        Config_save();

        String out = "✔ Pinch threshold set to ";
        out += String(Config_getPinchThreshold());
        out += " mA.";

        bot.sendMessage(msg, out.c_str(), kbd);
        return;
    }

    if (lower.startsWith("/setpinch")) {
        int space = lower.indexOf(' ');
        if (space < 0) {
            bot.sendMessage(msg, "⛔ Usage: /setpinch 900", kbd);
            return;
        }

        String numStr = lower.substring(space + 1);
        numStr.trim();

        long parsed = strtol(numStr.c_str(), &end, 10);
        if (*end != '\0' || parsed < 200 || parsed > 2000) {
            bot.sendMessage(msg, "⛔ Invalid value. Must be 200–2000 mA.", kbd);
            return;
        }

        Config_setPinchThreshold(parsed);
        Config_save();

        String out = "✔ Pinch threshold set to ";
        out += String(Config_getPinchThreshold());
        out += " mA.";

        bot.sendMessage(msg, out.c_str(), kbd);
        return;
    }

    bot.sendMessage(msg, "❓ Unknown command.\nType a number between 200 and 2000.", kbd);
}

// ---------------------------------------------------------
//  MOTOR TIMEOUT
// ---------------------------------------------------------
void handleMotorTimeout(TBMessage &msg) {
    menuState = MENU_SETTINGS;

    ReplyKeyboard kbd = buildSettingsKeyboard();

    String text = msg.text;
    text.trim();
    String lower = text;
    lower.toLowerCase();

    if (lower == "back" || lower == "/back") {
        handleSettings(msg);
        return;
    }

    if (lower == "motor timeout" || lower == "/setmotortime") {
        bot.sendMessage(
            msg,
            "⏱ Motor Timeout\n\n"
            "Type a number between 10 and 120 seconds.\n"
            "Example: 45",
            kbd
        );
        return;
    }

    char *end;
    long val = strtol(lower.c_str(), &end, 10);

    if (*end == '\0') {
        if (val < 10 || val > 120) {
            bot.sendMessage(msg, "⛔ Value must be between 10 and 120.", kbd);
            return;
        }

        Config_setMotorTimeout(val);
        Config_save();

        String out = "✔ Motor timeout set to ";
        out += String(Config_getMotorTimeout());
        out += " seconds.";

        bot.sendMessage(msg, out.c_str(), kbd);
        return;
    }

    if (lower.startsWith("/setmotortime")) {
        int space = lower.indexOf(' ');
        if (space < 0) {
            bot.sendMessage(msg, "⛔ Usage: /setmotortime 45", kbd);
            return;
        }

        String numStr = lower.substring(space + 1);
        numStr.trim();

        long parsed = strtol(numStr.c_str(), &end, 10);
        if (*end != '\0' || parsed < 10 || parsed > 120) {
            bot.sendMessage(msg, "⛔ Invalid value. Must be 10–120.", kbd);
            return;
        }

        Config_setMotorTimeout(parsed);
        Config_save();

        String out = "✔ Motor timeout set to ";
        out += String(Config_getMotorTimeout());
        out += " seconds.";

        bot.sendMessage(msg, out.c_str(), kbd);
        return;
    }

    bot.sendMessage(msg, "❓ Unknown command.\nType a number between 10 and 120.", kbd);
}

// ---------------------------------------------------------
//  RESET ENERGY
// ---------------------------------------------------------
void handleResetEnergy(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();

    Config_save();
    bot.sendMessage(msg, "🔋 Energy history reset", kbd);
}

// ---------------------------------------------------------
//  REBOOT
// ---------------------------------------------------------
void handleReboot(TBMessage &msg) {
    bot.sendMessage(msg, "🔄 Rebooting...");
    delay(1000);
    ESP.restart();
}

// ---------------------------------------------------------
//  HELP
// ---------------------------------------------------------
void handleHelp(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();

    String out = "📖 *HELP GUIDE*\n━━━━━━━━━━━━━━━\n";
    out += "/status – Full system report\n";
    out += "/health – System diagnostics\n";
    out += "/energy – 30‑day usage chart\n";
    out += "/logs – Recent activity\n";
    out += "/open – Open the door\n";
    out += "/close – Close the door\n";
    out += "/auto – Return to auto mode\n";
    out += "/settings – Adjust offsets & timezone\n";
    out += "/resetenergy – Reset energy history\n";
    out += "/reboot – Restart the controller\n\n";
    out += "📍 Send your location to update sunrise/sunset times\n\n";

    bot.sendMessage(msg, out.c_str(), kbd);
}

// ---------------------------------------------------------
//  UNKNOWN
// ---------------------------------------------------------
void handleUnknown(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();
    String reply = "❓ Unknown command: '" + msg.text + "'\nTry /help";
    bot.sendMessage(msg, reply.c_str(), kbd);
}

// ---------------------------------------------------------
//  DEBUG DOOR
// ---------------------------------------------------------
void handleDebugDoor(TBMessage &msg) {
    float current = Motor_getCurrent();

    String out;
    out.reserve(400);

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

    bot.sendMessage(msg, out.c_str());
}

// ---------------------------------------------------------
//  DEBUG TIME
// ---------------------------------------------------------
void handleDebugTime(TBMessage &msg) {
    time_t now = time(nullptr);
    struct tm* local = localtime(&now);
    struct tm* utc   = gmtime(&now);

    String out = "🛠 *DEBUG TIME*\n━━━━━━━━━━━━━━\n";

    if (!local) {
        out += "RTC not valid yet\n";
        bot.sendMessage(msg, out.c_str());
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

    int tzHours = TimeUtils_getUTCOffsetHours();
    out += "⏳ TZ Offset: " + String(tzHours) + "h\n";
    out += "🕰 DST Active: " + String(local->tm_isdst ? "yes" : "no") + "\n\n";

    out += "⏱ Uptime: " + TimeUtils_getUptime() + "\n";

    bot.sendMessage(msg, out.c_str());
}

// ---------------------------------------------------------
//  DEBUG SUN
// ---------------------------------------------------------
void handleDebugSun(TBMessage &msg) {
    if (!TimeUtils_timeIsValid()) {
        bot.sendMessage(msg, "⛔ Time not synced yet. Waiting for NTP...");
        return;
    }

    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);

    if (!ptm) {
        bot.sendMessage(msg, "Time not valid yet");
        return;
    }

    int year  = ptm->tm_year + 1900;
    int month = ptm->tm_mon + 1;
    int day   = ptm->tm_mday;

    int tzHours = TimeUtils_getUTCOffsetHours();

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

    bot.sendMessage(msg, out.c_str());
}

// ---------------------------------------------------------
//  DEBUG AUTO
// ---------------------------------------------------------
void handleDebugAuto(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();

    String logicState = "";
    switch (Motor_getState()) {
        case M_OPEN:    logicState = "OPEN"; break;
        case M_CLOSED:  logicState = "CLOSED"; break;
        case M_OPENING: logicState = "OPENING"; break;
        case M_CLOSING: logicState = "CLOSING"; break;
        case M_STUCK:   logicState = "STUCK"; break;
    }

    String out = "";
    out += "AUTO DEBUG\n";
    out += "Logic State: " + logicState + "\n";
    out += "remoteOverride: " + String(remoteOverride ? "true" : "false") + "\n";

    bot.sendMessage(msg, out.c_str(), kbd);
}

// ---------------------------------------------------------
//  DEBUG STATE
// ---------------------------------------------------------
void handleDebugState(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();

    String out = "STATE DEBUG\n";

    switch (Motor_getState()) {
        case M_OPEN:    out += "Door State: OPEN\n"; break;
        case M_CLOSED:  out += "Door State: CLOSED\n"; break;
        case M_OPENING: out += "Door State: OPENING\n"; break;
        case M_CLOSING: out += "Door State: CLOSING\n"; break;
        case M_STUCK:   out += "Door State: STUCK\n"; break;
    }

    out += "remoteOverride: " + String(remoteOverride ? "true" : "false") + "\n";

    bot.sendMessage(msg, out.c_str(), kbd);
}

// ---------------------------------------------------------
//  DEBUG LIMITS
// ---------------------------------------------------------
void handleDebugLimits(TBMessage &msg) {
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

    bot.sendMessage(msg, out.c_str());
}

// ---------------------------------------------------------
//  DEBUG ENERGY
// ---------------------------------------------------------
void handleDebugEnergy(TBMessage &msg) {
    String out;
    out.reserve(300);

    out += "🛠 *DEBUG ENERGY*\n";
    out += "━━━━━━━━━━━━━━━\n";
    out += "Total Used mAh:    " + String(Energy_getTodaymAh(), 2) + "\n";

    bot.sendMessage(msg, out.c_str());
}

// ---------------------------------------------------------
//  DEBUG ALL
// ---------------------------------------------------------
void handleDebugAll(TBMessage &msg) {
    ReplyKeyboard kbd = buildMainKeyboard();

    String out = "FULL DEBUG\n";

    switch (Motor_getState()) {
        case M_OPEN:    out += "Door State: OPEN\n"; break;
        case M_CLOSED:  out += "Door State: CLOSED\n"; break;
        case M_OPENING: out += "Door State: OPENING\n"; break;
        case M_CLOSING: out += "Door State: CLOSING\n"; break;
        case M_STUCK:   out += "Door State: STUCK\n"; break;
    }

    out += "remoteOverride: " + String(remoteOverride ? "true" : "false") + "\n";

    bot.sendMessage(msg, out.c_str(), kbd);
}

// ---------------------------------------------------------
//  DEBUG MENU
// ---------------------------------------------------------
void handleDebugMenu(TBMessage &msg) {
    menuState = MENU_DEBUG;

    ReplyKeyboard kbd = buildDebugKeyboard();

    String text =
        "🛠 DEBUG MENU\n"
        "━━━━━━━━━━━━━━\n"
        "Choose a debug function:";

    bot.sendMessage(msg, text.c_str(), kbd);
}

// ---------------------------------------------------------
//  DEBUG ON/OFF
// ---------------------------------------------------------
void handleDebugOn(TBMessage &msg) {
    debugMenuEnabled = true;

    ReplyKeyboard kbd = buildMainKeyboard();
    bot.sendMessage(msg, "🛠 Debug menu ENABLED", kbd);
}

void handleDebugOff(TBMessage &msg) {
    debugMenuEnabled = false;

    ReplyKeyboard kbd = buildMainKeyboard();
    bot.sendMessage(msg, "🛠 Debug menu DISABLED", kbd);
}

// ---------------------------------------------------------
//  DEBUG CONFIG
// ---------------------------------------------------------
void handleDebugConfig(TBMessage &msg) {
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

    bot.sendMessage(msg, out.c_str());
}
