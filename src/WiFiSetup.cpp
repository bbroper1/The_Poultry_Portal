#include "WiFiSetup.h"
#include "Config.h"
#include "Globals.h"
#include <WiFiManager.h>
#include <Arduino.h>

bool WiFiSetup_begin() {
    WiFiManager wm;

    // Handle factory reset
    if (factoryResetRequested) { 
        Serial.println("🧹 WiFiManager: resetting saved settings"); 
        wm.resetSettings();
    }

    wm.setConnectTimeout(20);
    wm.setConfigPortalTimeout(180);

    // Telegram parameters
    WiFiManagerParameter custom_botToken(
        "botToken",
        "Telegram Bot API Token",
        Config_getBotToken().c_str(),
        128
    );

    WiFiManagerParameter custom_chatId(
        "chatId",
        "Telegram Chat ID",
        Config_getChatID().c_str(),
        32
    );

    wm.addParameter(&custom_botToken);
    wm.addParameter(&custom_chatId);

    // Start WiFiManager
    if (!wm.autoConnect("PoultryPortal_AP")) {
        Serial.println("❌ WiFi connection failed");
        return false;
    }

    Serial.println("✅ WiFi connected");

    // Save Telegram settings
    String newBotToken = custom_botToken.getValue();
    String newChatId   = custom_chatId.getValue();

    if (newBotToken.length() > 0) {
        Config_setBotToken(newBotToken);
    }

    if (newChatId.length() > 0) {
        Config_setChatID(newChatId);
    }

    Config_save();

    return true;
}
