#pragma once
#include "modules/telegram/TelegramEvent.h"
#include "modules/telegram/TelegramClient.h"

namespace ConfigHandler {
    void handle(const TelegramEvent& evt, TelegramClient* client);
}