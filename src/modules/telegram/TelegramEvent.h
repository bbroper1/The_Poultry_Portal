#pragma once

#include <Arduino.h>
#include <stdint.h>

// ---------------------------------------------------------
// Update struct used by TelegramTask
// ---------------------------------------------------------
struct TelegramUpdate {
    String text;
    String chatId;
    float  latitude;
    float  longitude;

    TelegramUpdate()
        : text(""),
          chatId(""),
          latitude(0.0f),
          longitude(0.0f)
    {}
};

// ---------------------------------------------------------
// Telegram → System event types
// ---------------------------------------------------------
enum TelegramEventType {
    EVT_NONE = 0,

    // Core info
    EVT_STATUS,
    EVT_HEALTH,
    EVT_ENERGY,
    EVT_LOGS,

    // Door / mode
    EVT_OPEN,
    EVT_CLOSE,
    EVT_AUTO,

    // NEW: Manual override duration selections
    EVT_OVERRIDE_15,
    EVT_OVERRIDE_30,
    EVT_OVERRIDE_60,
    EVT_OVERRIDE_SUNSET,
    EVT_OVERRIDE_SUNRISE,
    EVT_OVERRIDE_CANCEL,

    // Menus / navigation
    EVT_SHOW_MAIN_MENU,
    EVT_SHOW_SETTINGS,
    EVT_SHOW_DEBUG,
    EVT_SHOW_TIMEZONE,
    EVT_SHOW_MOTOR_MENU,

    // Help
    EVT_HELP,

    // Simulation (dev)
    EVT_SIM_ON,
    EVT_SIM_OFF,
    EVT_SIM_STATUS,
    EVT_SIM_TEST,
    EVT_SIM_JAM,

    // Token and Chat ID (dev)
    EVT_SET_TOKEN,
    EVT_SET_CHAT,

    // Location pin drop
    EVT_LOCATION,

    // Timezone
    EVT_SET_TIMEZONE,
    EVT_TIMEZONE_OTHER_EUROPE,

    // Offsets
    EVT_SET_OPEN_OFFSET,
    EVT_SET_CLOSE_OFFSET,
    EVT_OFFSET_VALUE,

    // Motor settings
    EVT_SET_MOTOR_TIMEOUT,
    EVT_SET_PINCH_THRESHOLD,
    EVT_MOTOR_VALUE,

    // Motor menu actions
    EVT_MOTOR_STATUS,
    EVT_MOTOR_TEST,
    EVT_MOTOR_STOP,
    EVT_MOTOR_RESET_HEALTH,
    EVT_MOTOR_FORCE_STUCK,

    // Debug menu actions
    EVT_DEBUG_DOOR,
    EVT_DEBUG_TIME,
    EVT_DEBUG_SUN,
    EVT_DEBUG_AUTO,
    EVT_DEBUG_STATE,
    EVT_DEBUG_LIMITS,
    EVT_DEBUG_ENERGY,
    EVT_DEBUG_ALL,
    EVT_DEBUG_CONFIG,
    EVT_DEBUG_ON,
    EVT_DEBUG_OFF,

    // Event logs
    EVT_SHOW_LOGS,
    EVT_LOGS_NEXT,
    EVT_LOGS_PREV,
    EVT_LOGS_FIRST,
    EVT_LOGS_LAST,

    // Back
    EVT_BACK,

    // Fallback
    EVT_UNKNOWN
};

// ---------------------------------------------------------
// TelegramEvent: a single parsed Telegram command
// ---------------------------------------------------------
struct TelegramEvent {
    TelegramEventType type;
    uint64_t          chatId;
    String            text;
    float             latitude;
    float             longitude;

    // -----------------------------------------------------
    // Normalize Unicode punctuation to ASCII
    // -----------------------------------------------------
    static String normalize(String s) {
        s.toLowerCase();
        s.replace("–", "-");
        s.replace("—", "-");
        s.replace("‑", "-");
        s.replace("＋", "+");
        s.replace("／", "/");
        while (s.indexOf("  ") >= 0) s.replace("  ", " ");
        s.trim();
        return s;
    }

    // -----------------------------------------------------
    // Inline command classifier
    // -----------------------------------------------------
    static TelegramEventType fromText(const String& txtRaw) {

        String txt = txtRaw;
        txt.trim();
        String n = normalize(txt);

        // BACK (universal)
        if (n == "back" || n == "🏠 back") return EVT_BACK;

        // Core info
        if (txt == "/status" || txt == "📊 Status")     return EVT_STATUS;
        if (txt == "/health" || txt == "🩺 Health")     return EVT_HEALTH;
        if (txt == "/energy" || txt == "⚡ Energy")     return EVT_ENERGY;
        if (txt == "/logs"   || txt == "📝 Logs")       return EVT_LOGS;

        // Menus
        if (txt == "/menu"     || txt == "📋 Menu")      return EVT_SHOW_MAIN_MENU;
        if (txt == "/settings" || txt == "⚙️ Settings")  return EVT_SHOW_SETTINGS;
        if (txt == "/debug"    || txt == "🛠 Debug Menu")return EVT_SHOW_DEBUG;
        if (txt == "/timezone" || txt == "🕒 Timezone")  return EVT_SHOW_TIMEZONE;
        if (txt == "🔧 Motor Menu")                      return EVT_SHOW_MOTOR_MENU;

        // Door control (now triggers override menu)
        if (txt == "/open"  || txt == "👐 Open")         return EVT_OPEN;
        if (txt == "/close" || txt == "🚪 Close")        return EVT_CLOSE;
        if (txt == "/auto"  || txt == "🤖 Auto")         return EVT_AUTO;

        // NEW: Override duration selections
        if (txt == "15 min")         return EVT_OVERRIDE_15;
        if (txt == "30 min")         return EVT_OVERRIDE_30;
        if (txt == "1 hour")         return EVT_OVERRIDE_60;
        if (txt == "Until Sunset")   return EVT_OVERRIDE_SUNSET;
        if (txt == "Until Sunrise")  return EVT_OVERRIDE_SUNRISE;
        if (txt == "Cancel Override")return EVT_OVERRIDE_CANCEL;

        // Simulation
        if (txt == "/sim on")         return EVT_SIM_ON;
        if (txt == "/sim off")        return EVT_SIM_OFF;
        if (txt == "/sim status")     return EVT_SIM_STATUS;
        if (txt == "/sim test")       return EVT_SIM_TEST;
        if (txt == "/sim jam")        return EVT_SIM_JAM;

        // Token + Chat ID
        if (txt.startsWith("/settoken")) return EVT_SET_TOKEN;
        if (txt.startsWith("/setchat"))  return EVT_SET_CHAT;

        // Offsets
        if (txt == "☀️ Open Offset")  return EVT_SET_OPEN_OFFSET;
        if (txt == "☀️ Close Offset") return EVT_SET_CLOSE_OFFSET;

        // Motor menu buttons
        if (txt == "🔧 Motor Status")      return EVT_MOTOR_STATUS;
        if (txt == "🔁 Motor Test")        return EVT_MOTOR_TEST;
        if (txt == "🛑 Stop Motor")        return EVT_MOTOR_STOP;
        if (txt == "🔄 Reset Health")      return EVT_MOTOR_RESET_HEALTH;
        if (txt == "⚠️ Force Stuck")       return EVT_MOTOR_FORCE_STUCK;

        // Motor settings
        if (txt == "⌛ Motor Timeout")     return EVT_SET_MOTOR_TIMEOUT;
        if (txt == "🐥 Pinch Threshold")   return EVT_SET_PINCH_THRESHOLD;

        // Debug menu buttons
        if (txt == "🚪 Door Debug")   return EVT_DEBUG_DOOR;
        if (txt == "⏱ Time Debug")   return EVT_DEBUG_TIME;
        if (txt == "🌅 Sun Debug")    return EVT_DEBUG_SUN;
        if (txt == "🤖 Auto Debug")   return EVT_DEBUG_AUTO;
        if (txt == "⚙️ State Debug")  return EVT_DEBUG_STATE;
        if (txt == "🔘 Limit Debug")  return EVT_DEBUG_LIMITS;
        if (txt == "⚡ Energy Debug") return EVT_DEBUG_ENERGY;
        if (txt == "📑 Full Debug")   return EVT_DEBUG_ALL;
        if (txt == "📝 Logs")         return EVT_SHOW_LOGS;
        if (txt == "Next ▶")          return EVT_LOGS_NEXT;
        if (txt == "◀ Prev")          return EVT_LOGS_PREV;
        if (txt == "⏮ First")         return EVT_LOGS_FIRST;
        if (txt == "Last ⏭")          return EVT_LOGS_LAST;

        // Debug config / toggle
        if (txt == "/debugon")  return EVT_DEBUG_ON;
        if (txt == "/debugoff") return EVT_DEBUG_OFF;
        if (txt == "/debugcfg") return EVT_DEBUG_CONFIG;

        // Europe Other
        if (txt == "Europe Other") return EVT_TIMEZONE_OTHER_EUROPE;

        // Numeric input (motor or offset)
        {
            bool numeric = true;
            for (size_t i = 0; i < txt.length(); i++) {
                if (!isDigit(txt[i]) && txt[i] != '-' && txt[i] != '+') {
                    numeric = false;
                    break;
                }
            }
            if (numeric) return EVT_MOTOR_VALUE;
        }

        // Help
        if (txt == "/help"  || txt == "🆘 Help") return EVT_HELP;

        // Timezone typed input
        String t = normalize(txt);

        // US
        if (t == "pacific (utc-8)"  || t == "pacific"  || t == "pst" || t == "utc-8" || t == "-8") return EVT_SET_TIMEZONE;
        if (t == "mountain (utc-7)" || t == "mountain" || t == "mst" || t == "utc-7" || t == "-7") return EVT_SET_TIMEZONE;
        if (t == "central (utc-6)"  || t == "central"  || t == "cst" || t == "utc-6" || t == "-6") return EVT_SET_TIMEZONE;
        if (t == "eastern (utc-5)"  || t == "eastern"  || t == "est" || t == "utc-5" || t == "-5") return EVT_SET_TIMEZONE;

        // Europe
        if (t == "uk / gmt (utc+0)" || t == "gmt" || t == "uk" || t == "london" || t == "utc" || t == "+0") return EVT_SET_TIMEZONE;
        if (t == "cet (utc+1)"      || t == "cet" || t == "paris" || t == "berlin" || t == "utc+1" || t == "+1") return EVT_SET_TIMEZONE;
        if (t == "eet (utc+2)"      || t == "eet" || t == "athens" || t == "utc+2" || t == "+2") return EVT_SET_TIMEZONE;

        // Australia
        if (t == "aest (utc+10)"    || t == "aest" || t == "sydney" || t == "melbourne" || t == "utc+10" || t == "+10") return EVT_SET_TIMEZONE;
        if (t == "acst (utc+9:30)"  || t == "acst" || t == "adelaide" || t == "utc+9:30" || t == "+9:30") return EVT_SET_TIMEZONE;
        if (t == "awst (utc+8)"     || t == "awst" || t == "perth" || t == "utc+8" || t == "+8") return EVT_SET_TIMEZONE;

        return EVT_UNKNOWN;
    }

    TelegramEvent()
        : type(EVT_NONE),
          chatId(0),
          text(""),
          latitude(0.0f),
          longitude(0.0f)
    {}

    TelegramEvent(TelegramEventType t,
                  uint64_t          c,
                  const String&     txt,
                  float             lat = 0.0f,
                  float             lon = 0.0f)
        : type(t),
          chatId(c),
          text(txt),
          latitude(lat),
          longitude(lon)
    {}
};
