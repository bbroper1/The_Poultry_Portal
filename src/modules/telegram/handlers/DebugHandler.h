#pragma once

#include "../../telegram/TelegramEvent.h"
#include "../../telegram/TelegramClient.h"

class DebugHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);

private:
    // Shared formatting helpers
    static String blockHeader(const String& emoji, const String& title);
    static String kv(const String& label, const String& value);
    static String formatDoorState(int state);
    static String formatBool(bool v, const String& yes = "YES", const String& no = "No");

    // Debug sections
    static void sendDebugMenu(uint64_t chatId, TelegramClient* client);
    static void debugDoor(uint64_t chatId, TelegramClient* client);
    static void debugTime(uint64_t chatId, TelegramClient* client);
    static void debugSun(uint64_t chatId, TelegramClient* client);
    static void debugAuto(uint64_t chatId, TelegramClient* client);
    static void debugState(uint64_t chatId, TelegramClient* client);
    static void debugLimits(uint64_t chatId, TelegramClient* client);
    static void debugEnergy(uint64_t chatId, TelegramClient* client);
    static void debugConfig(uint64_t chatId, TelegramClient* client);
    static void debugAll(uint64_t chatId, TelegramClient* client);
};
