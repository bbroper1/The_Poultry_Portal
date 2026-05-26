#pragma once
#include <Arduino.h>
#include <stdint.h>

// ---------------------------------------------------------
// Outgoing Telegram message
// Used by SupervisorTask, handlers, and TelegramTask
// ---------------------------------------------------------
struct TelegramOutMessage {
    uint64_t chatId = 0;
    String   text;

    TelegramOutMessage() = default;

    TelegramOutMessage(uint64_t id, const String& t)
        : chatId(id), text(t)
    {}
};
