#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "TelegramClient.h"
#include "TelegramMessage.h"

extern TaskHandle_t s_telegramTaskHandle;

class TelegramTask {
public:
    explicit TelegramTask(TelegramClient* client);
    void start();

private:
    TelegramClient* client;

    static void taskEntry(void* pv);
    void run();
};