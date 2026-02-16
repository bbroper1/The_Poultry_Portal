#include "TelegramKeyboards.h"

// ---------------------------------------------------------
// Wrap raw keyboard array into Telegram reply_markup object
// ---------------------------------------------------------
String wrapKeyboard(const String& rawArray) {
    String out = "{\"keyboard\":";
    out += rawArray;
    out += ",\"resize_keyboard\":true}";
    return out;
}

// ---------------------------------------------------------
// MAIN MENU
// ---------------------------------------------------------
String kbMain() {
    menuState = MENU_MAIN;

    const __FlashStringHelper* raw =
        F("["
          "[\"📊 Status\",\"🩺 Health\"],"
          "[\"⚡ Energy\",\"📝 Logs\"],"
          "[\"⚙️ Settings\",\"🆘 Help\"],"
          "[\"👐 Open\",\"🤖 Auto\",\"🚪 Close\"]"
          "]");

    return wrapKeyboard(String(raw));
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

    // Optional Debug Menu
    if (debugMenuEnabled) {
        kbd += ",[\"🛠 Debug Menu\"]";
    }

    // Back
    kbd += ",[\"🏠 BACK\"]";

    kbd += "]";

    return wrapKeyboard(kbd);
}

// ---------------------------------------------------------
// MOTOR MENU
// ---------------------------------------------------------
String kbMotorMenu() {
    menuState = MENU_MOTOR;

    const __FlashStringHelper* raw =
        F("["
          "[\"🔧 Motor Status\",\"🔁 Motor Test\",\"🛑 Stop Motor\"],"
          "[\"⌛ Motor Timeout\",\"🐥 Pinch Threshold\"],"
          "[\"🔄 Reset Health\",\"⚠️ Force Stuck\"],"
          "[\"🏠 BACK\"]"
          "]");

    return wrapKeyboard(String(raw));
}

// ---------------------------------------------------------
// DEBUG MENU
// ---------------------------------------------------------
String kbDebug() {
    menuState = MENU_DEBUG;

    const __FlashStringHelper* raw =
        F("["
          "[\"🚪 Door Debug\",\"⏱ Time Debug\"],"
          "[\"🌅 Sun Debug\",\"🤖 Auto Debug\"],"
          "[\"⚙️ State Debug\",\"🔘 Limit Debug\"],"
          "[\"⚡ Energy Debug\",\"📑 Full Debug\"],"
          "[\"🏠 BACK\"]"
          "]");

    return wrapKeyboard(String(raw));
}

// ---------------------------------------------------------
// TIMEZONE MENU
// ---------------------------------------------------------
String kbTimezone() {
    menuState = MENU_TIMEZONE;

    const __FlashStringHelper* raw =
        F("["
          "[\"Pacific (UTC-8)\", \"Mountain (UTC-7)\"],"
          "[\"Central (UTC-6)\", \"Eastern (UTC-5)\"],"
          "[\"UK / GMT (UTC+0)\", \"CET (UTC+1)\"],"
          "[\"EET (UTC+2)\", \"Europe Other\"],"
          "[\"AEST (UTC+10)\", \"ACST (UTC+9:30)\"],"
          "[\"AWST (UTC+8)\", \"Australia Other\"],"
          "[\"BACK\"]"
          "]");

    return wrapKeyboard(String(raw));
}
// ---------------------------------------------------------
// LOGS PAGINATION KEYBOARD (First / Prev / Next / Last)
// ---------------------------------------------------------
String kbLogs(int page, int maxPage) {

    String kbd = "[";

    // Row 1: First / Prev / Next / Last
    kbd += "[";

    // First
    if (page > 0)
        kbd += "\"⏮ First\",";
    else
        kbd += "\" \",";   // disabled placeholder

    // Prev
    if (page > 0)
        kbd += "\"◀ Prev\",";
    else
        kbd += "\" \",";

    // Next
    if (page < maxPage)
        kbd += "\"Next ▶\",";
    else
        kbd += "\" \",";

    // Last
    if (page < maxPage)
        kbd += "\"Last ⏭\"";
    else
        kbd += "\" \"";

    kbd += "],";

    // Row 2: Back
    kbd += "[\"🏠 BACK\"]";

    kbd += "]";

    return wrapKeyboard(kbd);
}