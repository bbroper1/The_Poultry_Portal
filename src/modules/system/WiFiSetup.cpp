#include "WiFiSetup.h"
#include "modules/system/Globals.h"
#include "modules/system/Logging.h"
#include "Config.h"

#include <WiFiManager.h>
#include <Arduino.h>

// ---------------------------------------------------------
// WiFi + Telegram configuration via WiFiManager
// ---------------------------------------------------------
bool WiFiSetup_begin() {
    WiFiManager wm;

    // -----------------------------------------------------
    // Handle factory reset request (from override switch)
    // -----------------------------------------------------
    if (factoryResetRequested) {
        addLog("🧹 WiFiManager: resetting saved settings");
        wm.resetSettings();
    }

    wm.setConnectTimeout(20);
    wm.setConfigPortalTimeout(180);

    // -----------------------------------------------------
    // Custom WiFiManager parameters (Bot Token + Chat ID)
    // -----------------------------------------------------
    static char botTokenBuf[256];
    static char chatIdBuf[64];

    // Load existing values from Config
    strlcpy(botTokenBuf, Config_getBotToken().c_str(), sizeof(botTokenBuf));
    strlcpy(chatIdBuf,   Config_getChatID().c_str(), sizeof(chatIdBuf));

    WiFiManagerParameter p_botToken(
        "botToken",
        "Telegram Bot Token",
        botTokenBuf,
        sizeof(botTokenBuf)
    );

    WiFiManagerParameter p_chatId(
        "chatId",
        "Telegram Chat ID",
        chatIdBuf,
        sizeof(chatIdBuf)
    );

    wm.addParameter(&p_botToken);
    wm.addParameter(&p_chatId);

    // -----------------------------------------------------
    // Start WiFiManager AP
    // -----------------------------------------------------
    if (!wm.autoConnect("PoultryPortal_AP")) {
        addLog("❌ WiFi connection failed");
        return false;
    }

    addLog("✅ WiFi connected");

    // -----------------------------------------------------
    // Save updated Telegram settings
    // -----------------------------------------------------
    String newBotToken = p_botToken.getValue();
    String newChatId   = p_chatId.getValue();

    if (newBotToken.length() > 0) {
        Config_setBotToken(newBotToken);
        addLog("Updated Bot Token via WiFi portal");
    }

    if (newChatId.length() > 0) {
        Config_setChatID(newChatId);
        addLog("Updated Chat ID via WiFi portal");
    }

    Config_save();

    return true;
}