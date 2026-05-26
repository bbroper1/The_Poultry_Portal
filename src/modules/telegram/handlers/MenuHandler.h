#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class MenuHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);

private:
    static String blockHeader(const String& emoji, const String& title);
};
