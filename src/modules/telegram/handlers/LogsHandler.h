#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class LogsHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);
};