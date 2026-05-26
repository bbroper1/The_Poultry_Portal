/************************************************************
 * POULTRY PORTAL — MAIN FIRMWARE (MODERN VERSION)
 ************************************************************/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include <Preferences.h>

#include "modules/system/Globals.h"
#include "modules/system/Logging.h"
#include "modules/system/SystemStatus.h"
#include "modules/system/WiFiSetup.h"

#include "modules/motor/MotorPins.h"
#include "modules/motor/MotorTask.h"

#include "modules/sensors/SensorTask.h"
#include "modules/energy/EnergyModule.h"
#include "modules/battery/BatteryModule.h"

#include "modules/scheduler/SchedulerTask.h"
#include "modules/scheduler/SunContext.h"

#include "modules/display/DisplayTask.h"
#include "modules/automode/AutoModeTask.h"
#include "modules/hardware/HardwarePins.h"
#include "modules/config/Config.h"

#include "modules/utils/TimeUtils.h"
#include "modules/time/TimeManager.h"

#include "modules/telegram/TelegramClient.h"
#include "modules/telegram/TelegramTask.h"
#include "modules/telegram/TelegramCertificate.h"

#include "modules/system/SupervisorTask.h"

#define WDT_TIMEOUT 60

static void portalLog(const String& msg) {
    Serial.println(msg);
    addLog(msg);
}

void setup() {
    Serial.begin(115200);
    delay(200);

    portalLog("🐔 PoultryPortal Starting...");

    // -----------------------------------------------------
    // Initialize globals
    // -----------------------------------------------------
    Globals_begin();

    // -----------------------------------------------------
    // Hardware init
    // -----------------------------------------------------
    HardwarePins_begin();
    Motor_begin();
    DisplayTask_begin();

    // -----------------------------------------------------
    // Load config
    // -----------------------------------------------------
    Config_load();
    addLog("Bot token = [" + Config_getBotToken() + "]");

    // -----------------------------------------------------
    // WiFi + Telegram config portal
    // -----------------------------------------------------
    if (!WiFiSetup_begin()) {
        portalLog("❌ WiFi setup failed — rebooting");
        delay(2000);
        ESP.restart();
    }

    // -----------------------------------------------------
    // Time sync (non-blocking)
    // -----------------------------------------------------
    TimeManager::begin();
    portalLog("⏱️ Time sync started");

    // -----------------------------------------------------
    // Telegram subsystem
    // -----------------------------------------------------
    static TelegramClient tgClient(Config_getBotToken(), TELEGRAM_CERTIFICATE_ROOT);
    static TelegramTask tgTask(&tgClient);
    tgTask.start();

    // -----------------------------------------------------
    // Subsystems
    // -----------------------------------------------------
    Battery_begin();
    Energy_begin();
    SensorTask_begin();
    SunContext_begin();
    SchedulerTask_begin();
    AutoMode_begin();

    // -----------------------------------------------------
    // OTA
    // -----------------------------------------------------
    ArduinoOTA.setHostname("PoultryPortal");
    ArduinoOTA.begin();

    // -----------------------------------------------------
    // AutoMode correction after boot
    // -----------------------------------------------------
    AutoMode_bootCorrection();

    portalLog("Free heap: " + String(ESP.getFreeHeap()));
    portalLog("Free PSRAM: " + String(ESP.getFreePsram()));

    // -----------------------------------------------------
    // Supervisor Task (strict safety mode)
    // -----------------------------------------------------
    SupervisorTask_begin();

    portalLog("=== BOOT COMPLETE ===");
}

void handleSerialCommands() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    // -----------------------------
    // SET TOKEN
    // -----------------------------
    if (cmd.startsWith("settoken ")) {
        String token = cmd.substring(9);
        token.trim();

        Config_setBotToken(token);

        Serial.println();
        Serial.println("====================================");
        Serial.println("✅ BOT TOKEN UPDATED");
        Serial.println("New Token:");
        Serial.println("[" + token + "]");
        Serial.println("====================================");
        Serial.println();

        return;
    }

    // -----------------------------
    // SET CHAT ID
    // -----------------------------
    if (cmd.startsWith("setchat ")) {
        String chat = cmd.substring(8);
        chat.trim();

        Config_setChatID(chat);

        Serial.println();
        Serial.println("====================================");
        Serial.println("✅ CHAT ID UPDATED");
        Serial.println("New Chat ID:");
        Serial.println("[" + chat + "]");
        Serial.println("====================================");
        Serial.println();

        return;
    }
}

void loop() {
    handleSerialCommands();
    esp_task_wdt_reset();
    ArduinoOTA.handle();
    delay(10);
}
