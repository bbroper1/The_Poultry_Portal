#include "WiFiSetup.h"

#include "modules/system/Globals.h"
#include "modules/system/Logging.h"
#include "modules/time/TimeManager.h"
#include "modules/scheduler/SunContext.h"
#include "Config.h"

#include <WiFiManager.h>
#include <Arduino.h>

bool WiFiSetup_begin() {
    WiFiManager wm;

    // Optional: silence WiFiManager debug spam
    wm.setDebugOutput(false);

    // -----------------------------------------------------
    // Factory Reset
    // -----------------------------------------------------
    if (factoryResetRequested) {
        addLog("🧹 Factory Reset: clearing WiFi + Telegram settings");

        wm.resetSettings();

        Config_setBotToken("");
        Config_setChatID("");
        Config_save();

        factoryResetRequested = false;
    }

    // -----------------------------------------------------
    // WiFiManager behavior tuning
    // -----------------------------------------------------
    wm.setConnectTimeout(20);
    wm.setConfigPortalTimeout(180);

    // Always show custom fields
    wm.setShowStaticFields(true);

    // -----------------------------------------------------
    // Custom WiFiManager parameters (Bot Token + Chat ID)
    // -----------------------------------------------------
    static char botTokenBuf[256];
    static char chatIdBuf[64];

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
    addLog("📡 Starting WiFi setup portal: PoultryPortal_AP");

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

    bool changed = false;

    if (newBotToken.length() > 0 && newBotToken != Config_getBotToken()) {
        Config_setBotToken(newBotToken);
        addLog("🔑 Updated Bot Token via WiFi portal");
        changed = true;
    }

    if (newChatId.length() > 0 && newChatId != Config_getChatID()) {
        Config_setChatID(newChatId);
        addLog("💬 Updated Chat ID via WiFi portal");
        changed = true;
    }

    if (changed) {
        Config_save();
        addLog("💾 Telegram config saved");
    } else {
        addLog("ℹ️ No Telegram config changes");
    }

    // -----------------------------------------------------
    // Sync NTP + refresh sunrise/sunset
    // -----------------------------------------------------
    TimeManager::syncNTP();
    SunContext_refresh();

    return true;
}
