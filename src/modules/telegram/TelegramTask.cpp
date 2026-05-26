#include "TelegramTask.h"
#include "TelegramHandler.h"
#include "modules/config/Config.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

TaskHandle_t s_telegramTaskHandle = nullptr;

// Static pointer for global access
static TelegramClient* s_client = nullptr;

TelegramTask::TelegramTask(TelegramClient* client)
    : _client(client)
{
    s_client = client;   // ⭐ Global pointer for SupervisorTask
}

TelegramClient* TelegramTask::client() {
    return s_client;
}

void TelegramTask::sendMessageToOwner(const String& text) {
    if (!s_client) return;

    String chat = Config_getChatID();
    if (chat.length() == 0) return;

    uint64_t id = strtoull(chat.c_str(), nullptr, 10);
    s_client->sendMessage(id, text);
}

void TelegramTask::start() {
    xTaskCreatePinnedToCore(
        TelegramTask::taskEntry,
        "TelegramTask",
        16384,
        this,
        1,
        &s_telegramTaskHandle,
        1
    );
}

void TelegramTask::taskEntry(void* pv) {
    static_cast<TelegramTask*>(pv)->run();
}

void TelegramTask::run() {
    // Apply timezone for this task
    setenv("TZ", Config_getTimezone().c_str(), 1);
    tzset();

    Serial.println(">>> TelegramTask started");

    while (true) {

        // ---------------------------------------------------------
        // 1. Poll Telegram for updates
        // ---------------------------------------------------------
        TelegramUpdate update;
        bool ok = _client->getNextUpdate(update);

        if (!ok) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // ---------------------------------------------------------
        // 2. Normalize update into TelegramEvent
        // ---------------------------------------------------------
        TelegramEvent evt;

        evt.chatId = strtoull(update.chatId.c_str(), nullptr, 10);
        evt.text   = update.text;

        // ---------------------------------------------------------
        // LOCATION FIRST
        // ---------------------------------------------------------
        if (update.latitude != 0.0f || update.longitude != 0.0f) {
            evt.type      = EVT_LOCATION;
            evt.latitude  = update.latitude;
            evt.longitude = update.longitude;

            Serial.println(">>> ROUTER: received LOCATION pin");
            Serial.println(">>> LAT = " + String(evt.latitude, 6));
            Serial.println(">>> LON = " + String(evt.longitude, 6));

            TelegramHandler::handle(evt, _client);
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // ---------------------------------------------------------
        // TEXT COMMAND
        // ---------------------------------------------------------
        evt.type = TelegramEvent::fromText(evt.text);

        Serial.println(">>> ROUTER: normalized cmd = [" + evt.text + "]");
        Serial.println(">>> ROUTER: publishing event type = " + String(evt.type));

        // ---------------------------------------------------------
        // 3. Dispatch to TelegramHandler
        // ---------------------------------------------------------
        TelegramHandler::handle(evt, _client);

        // ---------------------------------------------------------
        // 4. Loop pacing
        // ---------------------------------------------------------
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
