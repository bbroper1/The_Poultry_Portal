 #pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class StatusHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);
};