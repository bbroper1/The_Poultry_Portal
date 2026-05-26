#pragma once
#include "../TelegramEvent.h"
#include "../TelegramClient.h"

class LogsHandler {
public:
    static void handle(const TelegramEvent& evt, TelegramClient* client);

private:
    static String blockHeader(const String& emoji, const String& title);
    static const int LOGS_PER_PAGE;
    static int currentPage;
};
