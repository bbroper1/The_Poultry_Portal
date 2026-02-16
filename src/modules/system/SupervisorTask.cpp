#include "modules/system/SupervisorTask.h"
#include "modules/system/SystemStatus.h"
#include "modules/system/Logging.h"
#include "modules/system/Globals.h"

#include "modules/motor/MotorTask.h"
#include "modules/battery/BatteryModule.h"
#include "modules/config/Config.h"
#include "modules/sensors/SensorModule.h"
#include "modules/energy/EnergyModule.h"
#include "modules/scheduler/SchedulerTask.h"
#include "modules/utils/MessageBus.h"

#include "modules/telegram/TelegramMessage.h"
#include "modules/telegram/TelegramTask.h"

#include "modules/automode/AutoModeTask.h"
#include "modules/display/DisplayTask.h"

#include "modules/time/TimeManager.h"   // ✅ FIXED — correct include

#include <WiFi.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ---------------------------------------------------------
// External task handles (declared in each module)
// ---------------------------------------------------------
extern TaskHandle_t s_motorTaskHandle;
extern TaskHandle_t s_autoModeTaskHandle;
extern TaskHandle_t s_schedulerTaskHandle;
extern TaskHandle_t s_telegramTaskHandle;
extern TaskHandle_t s_displayTaskHandle;
extern TaskHandle_t s_sensorTaskHandle;

// Supervisor task handle
TaskHandle_t s_supervisorTaskHandle = nullptr;

// ---------------------------------------------------------
// Helper: Check if a task is alive
// ---------------------------------------------------------
static bool taskAlive(TaskHandle_t h) {
    if (!h) return false;
    eTaskState st = eTaskGetState(h);
    return (st != eDeleted && st != eInvalid);
}

// ---------------------------------------------------------
// Send alert via Telegram subsystem
// ---------------------------------------------------------
static void supervisorAlert(const String& msg) {
    TelegramOutMessage out;
    out.chatId = strtoull(Config_getChatID().c_str(), nullptr, 10);
    out.text = msg;
    MessageBus::publish("telegram/out", out);
}

// ---------------------------------------------------------
// Supervisor Task
// ---------------------------------------------------------
static void SupervisorTask(void* pv) {
    const TickType_t tick = pdMS_TO_TICKS(2000); // every 2 seconds

    for (;;) {
        SystemStatus s = SystemStatus_get();

        // -----------------------------------------------------
        // 1. Task health monitoring
        // -----------------------------------------------------
        if (!taskAlive(s_motorTaskHandle)) {
            addLog("❌ MotorTask died");
            supervisorAlert("❌ *MotorTask crashed!* Rebooting...");
            ESP.restart();
        }

        if (!taskAlive(s_sensorTaskHandle)) {
            addLog("❌ SensorTask died");
            supervisorAlert("❌ *SensorTask crashed!* Rebooting...");
            ESP.restart();
        }

        if (!taskAlive(s_autoModeTaskHandle)) {
            addLog("⚠️ AutoModeTask died");
            supervisorAlert("⚠️ *AutoModeTask crashed!* Restarting...");
            vTaskDelete(s_autoModeTaskHandle);
            AutoMode_begin();
        }

        if (!taskAlive(s_schedulerTaskHandle)) {
            addLog("⚠️ SchedulerTask died");
            supervisorAlert("⚠️ *SchedulerTask crashed!* Restarting...");
            vTaskDelete(s_schedulerTaskHandle);
            SchedulerTask_begin();
        }

        if (!taskAlive(s_displayTaskHandle)) {
            addLog("⚠️ DisplayTask died");
            supervisorAlert("⚠️ *DisplayTask crashed!* Restarting...");
            vTaskDelete(s_displayTaskHandle);
            DisplayTask_begin();
        }

        // TelegramTask is object-owned — do NOT restart it
        if (!taskAlive(s_telegramTaskHandle)) {
            addLog("⚠️ TelegramTask not running");
            supervisorAlert("⚠️ *TelegramTask not running!* Check WiFi/TLS.");
        }

        // -----------------------------------------------------
        // 2. Battery safety
        // -----------------------------------------------------
        if (Battery_isCritical()) {
            addLog("🔴 CRITICAL BATTERY — Motor stopped");
            Motor_stop();
            supervisorAlert("🔴 *CRITICAL BATTERY!* Motor stopped.");
        }

        // -----------------------------------------------------
        // 3. Temperature safety
        // -----------------------------------------------------
        if (Sensor_getTemperature() > 60) {
            addLog("🔥 CRITICAL TEMP — Motor stopped");
            Motor_stop();
            supervisorAlert("🔥 *CRITICAL TEMPERATURE!* Motor stopped.");
        }

        // -----------------------------------------------------
        // 4. Motor stuck detection
        // -----------------------------------------------------
        if (s.doorState == M_STUCK) {
            supervisorAlert("⚠️ *Motor STUCK detected!*");
        }

        // -----------------------------------------------------
        // 5. WiFi health
        // -----------------------------------------------------
        if (!WiFi.isConnected()) {
            addLog("⚠️ WiFi disconnected");
        }

        // -----------------------------------------------------
        // 6. Time validity
        // -----------------------------------------------------
        if ((millis() / 1000) > 60 && !TimeManager::isValid()) {
            addLog("⚠️ Time invalid — NTP sync failed");
            supervisorAlert("⚠️ *Time invalid — NTP sync failed*");
        }

        vTaskDelay(tick);
    }
}

// ---------------------------------------------------------
// Public API
// ---------------------------------------------------------
void SupervisorTask_begin() {
    if (!s_supervisorTaskHandle) {
        xTaskCreatePinnedToCore(
            SupervisorTask,
            "SupervisorTask",
            4096,
            nullptr,
            3,      // high priority (below MotorTask)
            &s_supervisorTaskHandle,
            1       // logic core
        );
        addLog("SupervisorTask → started");
    }
}