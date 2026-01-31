#include "Globals.h"
#include "Config.h"
#include "Scheduler.h"
#include "TelegramRouter.h"
#include "TelegramHandlers.h"
#include "Logging.h"
#include "Energy.h"
#include "Battery.h"
#include "Motor.h"
#include "StringUtils.h"

#include <WiFi.h>
#include <WebSerial.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ---------------------------------------------------------
//  GLOBAL BOT + CLIENT
// ---------------------------------------------------------
//WiFiClientSecure secured_client;
//UniversalTelegramBot bot("", secured_client);   
// ---------------------------------------------------------
//  KEYBOARD BUILDERS (Cleaned for UniversalTelegramBot)
// ---------------------------------------------------------
String kbMain() {
    menuState = MENU_MAIN;
    // Remove the {"keyboard": ... } wrapper
    return F("["
             "[\"📊 Status\",\"🩺 Health\"],"
             "[\"⚡ Energy\",\"📝 Logs\"],"
             "[\"⚙️ Settings\",\"🆘 Help\"],"
             "[\"👐 Open\",\"🤖 Auto\",\"🚪 Close\"]"
             "]");
}
String kbSettings() {
    menuState = MENU_SETTINGS;
    
    // Start the keyboard array
    String kbd = "[";
    
    // Row 1: Timezone and the special Request Location button
    // Note: We use { "text": "...", "request_location": true } for the GPS trigger
    kbd += "[\"🕒 Timezone\", {\"text\":\"📍 Location\", \"request_location\": true}],";
    
    // Row 2: Offsets
    kbd += "[\"☀️ Open Offset\",\"☀️ Close Offset\"],";
    
    // Row 3: Thresholds
    kbd += "[\"⌛ Motor Timeout\",\"🐥 Pinch Threshold\"]";
    
    // Only add the Debug button row if enabled
    if (debugMenuEnabled) {
        kbd += ",[\"Debug Menu\"]"; 
    }
    
    // Final Row: Back
    kbd += ",[\"🏠 BACK\"]";
    kbd += "]";
    
    return kbd;
}

String kbDebug() {
    menuState = MENU_DEBUG;
    return F("["
             "[\"Test Mode ON\",\"Test Mode OFF\"],"
             "[\"Debug Door\",\"Debug Limits\",\"Debug Energy\"],"
             "[\"Debug Time\",\"Debug Sun\",\"Debug Auto\"],"
             "[\"Debug State\",\"Debug Config\",\"Debug All\"],"
             "[\"Reboot\",\"Debug Off\"],"
             "[\"🏠 BACK\"]"
             "]");
}
String kbTimezone() {
    return F("["
             "[\"Pacific (-8)\", \"Mountain (-7)\"],"
             "[\"Central (-6)\", \"Eastern (-5)\"],"
             "[\"BACK\"]"
             "]");
}
// ---------------------------------------------------------
//  SEND MESSAGE WRAPPER (Updated for UniversalBot Signature)
// ---------------------------------------------------------
void sendMessageWithKeyboard(String chat_id, const String &text, const String &kbd) {
    bot.sendMessageWithReplyKeyboard(chat_id, text, "Markdown", kbd, true);
}

// ---------------------------------------------------------
//  MAIN ROUTER 
// ---------------------------------------------------------
void TelegramRouter_handle() {
    static unsigned long lastPollTime = 0;
    const unsigned long pollInterval = 2000; 

    if (millis() - lastPollTime < pollInterval) return;
    lastPollTime = millis();

    if (millis() - bootTime < 8000 || !telegramEnabled) return;
    if (WiFi.status() != WL_CONNECTED) return;

    // 1. Get updates
    int numNew = bot.getUpdates(bot.last_message_received + 1);

    for (int i = 0; i < numNew; i++) {
        TBMessage msg = bot.messages[i];
        
        // Update the ID immediately so we never process this specific message again
        bot.last_message_received = msg.update_id;

        // --- 1. PIN CATCHER ---
        if (msg.type == "location" || msg.latitude != 0.0f) {
            handleLocation(msg); 
            continue; 
        }

        // --- 2. SECURITY CHECK ---
        String authorizedID = Config_getChatID();
        if (authorizedID.length() > 0 && authorizedID != "0" && msg.chat_id != authorizedID) {
            continue; 
        }

        // --- 3. FUNCTIONAL LOCKOUT ---
        // If door is moving, tell user and STOP processing this message.
        if (Motor_getState() == M_OPENING || Motor_getState() == M_CLOSING) {
            bot.sendMessage(msg.chat_id, "⚙️ Door is moving — please wait.", "");
            continue;
        }

        // --- 4. PREPARE COMMAND ---
        userid = atoll(msg.chat_id.c_str());
        String text = msg.text;
        text.trim();
        String cmd = text;
        cmd.toLowerCase();

        // --- 5. MAIN ROUTING ---
        if (cmd == "📊 status" || cmd == "/status" || cmd == "/start") handleStatus(msg);
        else if (cmd == "⚡ energy" || cmd == "/energy") handleEnergy(msg);
        else if (cmd == "📝 logs" || cmd == "/logs")   handleLogs(msg);
        else if (cmd == "👐 open" || cmd == "/open")   handleOpen(msg);
        else if (cmd == "🚪 close" || cmd == "/close")  handleClose(msg);
        else if (cmd == "🤖 auto" || cmd == "/auto")   handleAuto(msg);
        else if (cmd == "🩺 health" || cmd == "/health") handleHealth(msg);
        else if (cmd == "⚙️ settings" || cmd == "/settings" || cmd == "settings") handleSettings(msg);
        else if (cmd == "🕒 timezone" || cmd == "/timezone") handleTimezone(msg);
        else if (cmd == "📍 location" || cmd == "/location") handleLocation(msg);
        
        else if (cmd == "☀️ open offset" || cmd == "/openoffset") {
            waitingForOpenOffset = true;
            waitingForCloseOffset = false;
            handleOffsets(msg);
        }
        else if (cmd == "☀️ close offset" || cmd == "/closeoffset") {
            waitingForOpenOffset = false;
            waitingForCloseOffset = true;
            handleOffsets(msg);
        }
        else if ((waitingForOpenOffset || waitingForCloseOffset) && StringUtils_isNumber(text)) {
            handleOffsets(msg);
        }
        else if (cmd == "⌛ motor timeout") handleMotorTimeout(msg);
        else if (cmd == "🐥 pinch threshold") handlePinchThreshold(msg);
        
        else if (cmd == "timezone" || cmd == "central" || cmd == "eastern" || cmd == "mountain" || cmd == "pacific") {
            handleTimezone(msg);
        }
        else if (cmd == "🏠 back" || cmd == "/back") {
            waitingForOpenOffset = false;
            waitingForCloseOffset = false;
            sendMessageWithKeyboard(msg.chat_id, "🏠 Main Menu", kbMain());
            menuState = MENU_MAIN;
        }
        // DEBUG COMMANDS
        else if (cmd == "debug door") handleDebugDoor(msg);
        else if (cmd == "debug limits") handleDebugLimits(msg);
        else if (cmd == "debug energy") handleDebugEnergy(msg);
        else if (cmd == "debug time")   handleDebugTime(msg);
        else if (cmd == "debug sun")    handleDebugSun(msg);
        else if (cmd == "debug auto")   handleDebugAuto(msg);
        else if (cmd == "debug state")  handleDebugState(msg);
        else if (cmd == "debug config")  handleDebugConfig(msg);
        else if (cmd == "debug all")    handleDebugAll(msg);
        else if (cmd == "debug off" || cmd == "/debugoff") handleDebugOff(msg);
        else if (cmd == "🆘 help" || cmd == "/help") handleHelp(msg);
        else if (cmd == "reboot" || cmd == "/reboot") handleReboot(msg);
        else if (cmd == "debug on" || cmd == "/debugon")  handleDebugOn(msg);
        else if (cmd == "debug menu" || cmd == "/debugmenu" || cmd == "debug") {
            if (debugMenuEnabled) handleDebugMenu(msg);
            else bot.sendMessage(msg.chat_id, "Debug menu is disabled.", "");
        }
        else if (cmd == "test mode on") {
            if (debugMenuEnabled) handleTestModeOn(msg);
        }
        else if (cmd == "test mode off") {
            if (debugMenuEnabled) handleTestModeOff(msg);
        }
        else {
            handleUnknown(msg);
        }
    }
}
// ---------------------------------------------------------
//  INITIALIZATION
// ---------------------------------------------------------
void TelegramRouter_init() {
    telegramEnabled = false;

    String token = Config_getBotToken();
    token.trim();
    if (token.length() < 10) return;

    secured_client.setInsecure();
    bot.updateToken(token);

    Serial.println("Connecting Telegram bot...");
    WebSerial.println("Connecting Telegram bot...");

    if (bot.sendMessageWithReplyKeyboard(
            Config_getChatID(),
            "System Online 🚀",
            "Markdown",
            kbMain(),
            true
        )) 
    {
        Serial.println(">>> SUCCESS! Keyboards are active.");
        WebSerial.println(">>> SUCCESS! Keyboards are active.");
        telegramEnabled = true;
    } 
    else 
    {
        Serial.println(">>> Telegram connected (Silent Mode).");
        WebSerial.println(">>> Telegram connected (Silent Mode).");
        telegramEnabled = true;
    }
}