#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "TelegramClient.h"
#include "TelegramEvent.h"

extern TaskHandle_t s_telegramTaskHandle;

class TelegramTask {
public:
    explicit TelegramTask(TelegramClient* client);
    void start();

    // Global accessor for SupervisorTask and handlers
    static TelegramClient* client();

    // Convenience helper for outbound alerts
    static void sendMessageToOwner(const String& text);

private:
    TelegramClient* _client;

    static void taskEntry(void* pv);
    void run();
};
