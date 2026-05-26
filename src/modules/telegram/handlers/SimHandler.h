#pragma once
#include "modules/telegram/TelegramEvent.h"
#include "modules/telegram/TelegramClient.h"

class SimHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);

private:
    static String blockHeader(const String& emoji, const String& title);
    static String kv(const String& label, const String& value);
};
