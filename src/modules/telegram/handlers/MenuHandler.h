#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class MenuHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);
};