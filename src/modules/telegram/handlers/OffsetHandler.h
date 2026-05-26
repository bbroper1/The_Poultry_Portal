#pragma once
#include "../../telegram/TelegramEvent.h"
#include "../../telegram/TelegramClient.h"

class OffsetHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);
    static bool isWaitingForOpen()  { return waitingForOpen; }
    static bool isWaitingForClose() { return waitingForClose; }


private:
    // Internal state
    static bool waitingForOpen;
    static bool waitingForClose;

    // Helpers
    static bool isInteger(const String& s);
    static String formatOffset(int val);
    static String blockHeader(const String& emoji, const String& title);
};
