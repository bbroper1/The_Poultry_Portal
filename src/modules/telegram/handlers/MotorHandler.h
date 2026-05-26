#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class MotorHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);

private:
    static bool isNumber(const String& s);
    static String blockHeader(const String& emoji, const String& title);
    static String kv(const String& label, const String& value);
    static String formatDoorState(int state);
    static void sendMotorStatus(uint64_t chatId, TelegramClient* client);
};
