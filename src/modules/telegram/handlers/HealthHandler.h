#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class HealthHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);

private:
    static String blockHeader(const String& emoji, const String& title);
    static String kv(const String& label, const String& value);
    static String formatTempLabel(float c);
    static String formatRemaining(time_t until);
};
