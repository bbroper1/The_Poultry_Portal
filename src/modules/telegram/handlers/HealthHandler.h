#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class HealthHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);
};