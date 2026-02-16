#pragma once

#include "modules/telegram/TelegramEvent.h"

// Forward declaration to avoid heavy includes in the header
class TelegramClient;

namespace TelegramHandler {

    // Public dispatcher for all Telegram events
    void handle(const TelegramEvent& evt, TelegramClient* client);

} // namespace TelegramHandler