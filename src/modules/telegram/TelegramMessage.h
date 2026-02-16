#pragma once
#include <Arduino.h>
#include <stdint.h>

// ---------------------------------------------------------
// Outgoing Telegram message
// Published to MessageBus channel: "telegram/out"
// Consumed by TelegramTask to send via HTTPS
// ---------------------------------------------------------
struct TelegramOutMessage {
    uint64_t chatId = 0;
    String   text;

    TelegramOutMessage() = default;

    TelegramOutMessage(uint64_t id, const String& t)
        : chatId(id), text(t)
    {}
};