#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class EnergyHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);

private:
    static String blockHeader(const String& emoji, const String& title);
    static String kv(const String& label, const String& value);
    static String makeSparkline(float* data, int len);
};
