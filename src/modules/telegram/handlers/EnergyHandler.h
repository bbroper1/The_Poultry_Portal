#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class EnergyHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);
};