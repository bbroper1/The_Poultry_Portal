#include "TelegramKeyboards.h"

// ---------------------------------------------------------
// Wrap raw keyboard array into Telegram reply_markup object
// ---------------------------------------------------------
String wrapKeyboard(const String& rawArray) {
    // Basic sanity check
    if (!rawArray.startsWith("[") || !rawArray.endsWith("]")) {
        return "{\"keyboard\":[],\"resize_keyboard\":true}";
    }

    String out;
    out.reserve(rawArray.length() + 40);
    out = "{\"keyboard\":";
    out += rawArray;
    out += ",\"resize_keyboard\":true}";
    return out;
}

// ---------------------------------------------------------
// MAIN MENU
// ---------------------------------------------------------
String kbMain() {
    menuState = MENU_MAIN;

    return wrapKeyboard(
        F("["
          "[\"📊 Status\",\"🩺 Health\"],"
          "[\"⚡ Energy\",\"📝 Logs\"],"
          "[\"⚙️ Settings\",\"🆘 Help\"],"
          "[\"👐 Open\",\"🤖 Auto\",\"🚪 Close\"]"
          "]")
    );
}

// ---------------------------------------------------------
// SETTINGS MENU
// ---------------------------------------------------------
String kbSettings() {
    menuState = MENU_SETTINGS;

    String kbd = "[";
    kbd += "[\"🕒 Timezone\", {\"text\":\"📍 Location\", \"request_location\": true}],";
    kbd += "[\"☀️ Open Offset\",\"☀️ Close Offset\"],";
    kbd += "[\"🔧 Motor Menu\"]";

    if (debugMenuEnabled)
        kbd += ",[\"🛠 Debug Menu\"]";

    kbd += ",[\"🏠 BACK\"]]";
    return wrapKeyboard(kbd);
}

// ---------------------------------------------------------
// MOTOR MENU
// ---------------------------------------------------------
String kbMotorMenu() {
    menuState = MENU_MOTOR;

    return wrapKeyboard(
        F("["
          "[\"🔧 Motor Status\",\"🔁 Motor Test\",\"🛑 Stop Motor\"],"
          "[\"⌛ Motor Timeout\",\"🐥 Pinch Threshold\"],"
          "[\"🔄 Reset Health\",\"⚠️ Force Stuck\"],"
          "[\"🏠 BACK\"]"
          "]")
    );
}

// ---------------------------------------------------------
// MANUAL OVERRIDE DURATION MENU
// ---------------------------------------------------------
String kbOverrideMenu(bool isOpen) {
    // menuState unchanged — modal menu

    String kbd = "[";
    kbd += "[\"15 min\", \"30 min\"],";
    kbd += "[\"1 hour\", \"Until Sunset\"],";
    kbd += "[\"Until Sunrise\"],";
    kbd += "[\"Cancel Override\"],";
    kbd += "[\"🏠 BACK\"]]";
    return wrapKeyboard(kbd);
}

// ---------------------------------------------------------
// DEBUG MENU
// ---------------------------------------------------------
String kbDebug() {
    menuState = MENU_DEBUG;

    return wrapKeyboard(
        F("["
          "[\"🚪 Door Debug\",\"⏱ Time Debug\"],"
          "[\"🌅 Sun Debug\",\"🤖 Auto Debug\"],"
          "[\"⚙️ State Debug\",\"🔘 Limit Debug\"],"
          "[\"⚡ Energy Debug\",\"📑 Full Debug\"],"
          "[\"🏠 BACK\"]"
          "]")
    );
}

// ---------------------------------------------------------
// TIMEZONE MENU
// ---------------------------------------------------------
String kbTimezone() {
    menuState = MENU_TIMEZONE;

    return wrapKeyboard(
        F("["
          "[\"Pacific (UTC-8)\", \"Mountain (UTC-7)\"],"
          "[\"Central (UTC-6)\", \"Eastern (UTC-5)\"],"
          "[\"UK / GMT (UTC+0)\", \"CET (UTC+1)\"],"
          "[\"EET (UTC+2)\", \"Europe Other\"],"
          "[\"AEST (UTC+10)\", \"ACST (UTC+9:30)\"],"
          "[\"AWST (UTC+8)\", \"Australia Other\"],"
          "[\"BACK\"]"
          "]")
    );
}

// ---------------------------------------------------------
// LOGS PAGINATION KEYBOARD
// ---------------------------------------------------------
String kbLogs(int page, int maxPage) {
    String kbd = "[";

    kbd += "[";

    // First
    kbd += (page > 0) ? "\"⏮ First\"," : "\"⛔\",";

    // Prev
    kbd += (page > 0) ? "\"◀ Prev\"," : "\"⛔\",";

    // Next
    kbd += (page < maxPage) ? "\"Next ▶\"," : "\"⛔\",";

    // Last
    kbd += (page < maxPage) ? "\"Last ⏭\"" : "\"⛔\"";

    kbd += "],";

    kbd += "[\"🏠 BACK\"]]";
    return wrapKeyboard(kbd);
}
