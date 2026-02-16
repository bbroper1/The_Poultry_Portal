#pragma once
#include "modules/telegram/TelegramEvent.h"
#include "modules/telegram/TelegramClient.h"

namespace SimHandler {
    void handle(const TelegramEvent& evt, TelegramClient* client);
}