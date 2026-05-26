#pragma once

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "TelegramEvent.h"

// ---------------------------------------------------------
// TelegramClient
// ---------------------------------------------------------
class TelegramClient {
public:
    TelegramClient(const String& token, const char* rootCert);

    // Send plain text message
    bool sendMessage(uint64_t chatId, const String& text);

    // Send message with custom keyboard
    bool sendMessageWithKeyboard(
        uint64_t chatId,
        const String& text,
        const String& keyboardJson
    );

    // Send photo by URL
    bool sendPhotoByUrl(uint64_t chatId, const String& url, const String& caption);

    // Polling API used by TelegramTask
    bool getNextUpdate(TelegramUpdate& out);

    // Raw update fetcher (JSON string)
    String getUpdates();

    // Build full Telegram API URL
    String buildURL(const String& method);

    // Offset tracking
    uint64_t getLastUpdateId() const { return lastUpdateId; }
    void setLastUpdateId(uint64_t id) { lastUpdateId = id; }

    // Global instance accessor (SupervisorTask uses this)
    static TelegramClient* getInstance() { return instance; }

private:
    WiFiClientSecure client;
    String token;

    uint64_t lastUpdateId = 0;

    bool httpsGET(const String& url, String& response);
    bool httpsPOST(const String& url, const String& body, String& response);

    // JSON escaping helper
    static String escapeJSON(const String& s);

    // Global instance pointer
    static TelegramClient* instance;
};
