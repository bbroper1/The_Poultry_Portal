#include "Globals.h"
#include "Config.h"
#include "Scheduler.h"
#include "TelegramRouter.h"
#include "TelegramHandlers.h"
#include "Logging.h"

#include <AsyncTelegram2.h>
#include <WiFi.h>
#include <time.h>

#include "Energy.h"
#include "Battery.h"
#include "Motor.h"
#include "StringUtils.h"

extern AsyncTelegram2 bot;

// ---------------------------------------------------------
//  TELEGRAM POLLING (non-blocking)
// ---------------------------------------------------------
static bool pollTelegram(TBMessage &msg) {
    unsigned long start = millis();
    while (millis() - start < 300) {
        if (bot.getNewMessage(msg)) return true;
        delay(5);
    }
    return false;
}

// ---------------------------------------------------------
//  KEYBOARD BUILDERS
// ---------------------------------------------------------
ReplyKeyboard buildMainKeyboard() {
    menuState = MENU_MAIN;

    ReplyKeyboard kbd;
    kbd.addButton("/status"); kbd.addButton("/health"); kbd.addRow();
    kbd.addButton("/energy"); kbd.addButton("/logs");   kbd.addRow();
    kbd.addButton("/settings"); kbd.addButton("/help"); kbd.addRow();
    kbd.addButton("/open");
    kbd.addButton("/auto"); kbd.addButton("/close");
    return kbd;
}

ReplyKeyboard buildSettingsKeyboard() {
    ReplyKeyboard kbd;
    kbd.addButton("OpenOffset");
    kbd.addButton("Location");
    kbd.addButton("CloseOffset");
    kbd.addRow();
    kbd.addButton("Motor Timeout");
    kbd.addButton("Pinch Threshold");
    kbd.addRow();
    kbd.addButton("Timezone");
    if (debugMenuEnabled) kbd.addButton("Debug");
    kbd.addRow();
    kbd.addButton("Back");
    return kbd;
}

ReplyKeyboard buildDebugKeyboard() {
    ReplyKeyboard kbd;
    kbd.addButton("/debugauto");  kbd.addButton("/debugdoor");   kbd.addRow();
    kbd.addButton("/debugsun");   kbd.addButton("/debugtime");   kbd.addRow();
    kbd.addButton("/debugstate"); kbd.addButton("/debuglimits"); kbd.addRow();
    kbd.addButton("/debugconfig");kbd.addButton("/debugenergy"); kbd.addRow();
    kbd.addButton("/debugall");   kbd.addButton("/back");
    return kbd;
}

// ---------------------------------------------------------
//  ROUTER
// ---------------------------------------------------------
void TelegramRouter_handle() {
    if (millis() - bootTime < 8000) return;
    if (!telegramEnabled) return;

    TBMessage msg;
    if (!pollTelegram(msg)) return;

    if (ignoreFirstTelegramMessage) {
        ignoreFirstTelegramMessage = false;
        return;
    }

    // Door moving lockout
    if (Motor_getState() == M_OPENING || Motor_getState() == M_CLOSING) {
        ReplyKeyboard kbd = buildMainKeyboard();
        bot.sendMessage(msg, "⚙️ Door is moving — please wait.", kbd);
        return;
    }

    userid = msg.sender.id;

    String text = msg.text;
    text.trim();
    text.toLowerCase();

    // LOCATION PIN HANDLING
    if (text.length() == 0 &&
        msg.location.latitude  != 0.0 &&
        msg.location.longitude != 0.0 &&
        abs(msg.location.latitude)  > 0.1 &&
        abs(msg.location.longitude) > 0.1) {

        Config_setLat(msg.location.latitude);
        Config_setLong(msg.location.longitude);
        Config_save();

        ReplyKeyboard kbd = buildMainKeyboard();
        String locMsg = "📍 Location Updated!\nLat: " + String(Config_getLat(), 4) +
                        "\nLong: " + String(Config_getLong(), 4);
        bot.sendMessage(msg, locMsg.c_str(), kbd);
        addLog("Location Updated 📍");
        return;
    }

    // TOP-LEVEL COMMANDS
    if (text == "/status" || text == "/start") handleStatus(msg);
    else if (text == "/energy") handleEnergy(msg);
    else if (text == "/logs") handleLogs(msg);
    else if (text == "/open") handleOpen(msg);
    else if (text == "/close") handleClose(msg);
    else if (text == "/auto") handleAuto(msg);
    else if (text == "/health") handleHealth(msg);
    else if (text == "/settings" || text == "settings") handleSettings(msg);

    // SETTINGS SUBMENU
    else if (text == "timezone") handleTimezone(msg);
    else if (text == "location") handleLocation(msg);

    // OFFSET COMMANDS
    else if (text == "openoffset") {
        waitingForOpenOffset = true;
        waitingForCloseOffset = false;
        handleOffsets(msg);
        return;
    }
    else if (text == "closeoffset") {
        waitingForOpenOffset = false;
        waitingForCloseOffset = true;
        handleOffsets(msg);
        return;
    }

    // OFFSET TYPED INPUT
    else if ((waitingForOpenOffset || waitingForCloseOffset) && StringUtils_isNumber(text)) {
        handleOffsets(msg);
        return;
    }

    // EXIT OFFSET MODE
    else if (waitingForOpenOffset || waitingForCloseOffset) {
        waitingForOpenOffset = false;
        waitingForCloseOffset = false;
    }

    // MOTOR TIMEOUT / PINCH
    else if (text == "motor timeout") handleMotorTimeout(msg);
    else if (text == "pinch threshold") handlePinchThreshold(msg);
    else if (text.startsWith("/setmotortime")) handleMotorTimeout(msg);
    else if (text.startsWith("/setpinch")) handlePinchThreshold(msg);

    // BACK
    else if (text == "back" || text == "/back") {
        waitingForOpenOffset = false;
        waitingForCloseOffset = false;

        if (menuState == MENU_SETTINGS) {
            ReplyKeyboard kbd = buildMainKeyboard();
            bot.sendMessage(msg, "🏠 Main Menu", kbd);
            menuState = MENU_MAIN;
            return;
        }

        if (menuState == MENU_TIMEZONE || menuState == MENU_DEBUG) {
            handleSettings(msg);
            return;
        }

        ReplyKeyboard kbd = buildMainKeyboard();
        bot.sendMessage(msg, "🏠 Main Menu", kbd);
        menuState = MENU_MAIN;
        return;
    }

    // TIMEZONE SLASH COMMAND
    else if (text == "/timezone" || text.startsWith("/timezone")) handleTimezone(msg);

    // ENERGY / REBOOT / HELP
    else if (text == "/resetenergy") handleResetEnergy(msg);
    else if (text == "/reboot") handleReboot(msg);
    else if (text == "/help") handleHelp(msg);

    // DEBUG TOGGLES
    else if (text == "/debugon") {
        debugMenuEnabled = true;
        bot.sendMessage(msg, "🛠 Debug menu enabled.");
    }
    else if (text == "/debugoff") {
        debugMenuEnabled = false;
        bot.sendMessage(msg, "🛠 Debug menu disabled.");
    }

    // DEBUG MENU
    else if (text == "/debug" || text == "debug") {
        if (debugMenuEnabled) handleDebugMenu(msg);
        else bot.sendMessage(msg, "Debug menu disabled. Use /debugon.");
    }

    // DEBUG COMMANDS
    else if (text == "/debugsun") handleDebugSun(msg);
    else if (text == "/debugtime") handleDebugTime(msg);
    else if (text == "/debugdoor") handleDebugDoor(msg);
    else if (text == "/debugauto") handleDebugAuto(msg);
    else if (text == "/debugstate") handleDebugState(msg);
    else if (text == "/debuglimits") handleDebugLimits(msg);
    else if (text == "/debugconfig") handleDebugConfig(msg);
    else if (text == "/debugenergy") handleDebugEnergy(msg);
    else if (text == "/debugall") handleDebugAll(msg);

    // UNKNOWN
    else handleUnknown(msg);
}

// ---------------------------------------------------------
//  INIT
// ---------------------------------------------------------
void TelegramRouter_init() {
    // Reserved for future setup
}
