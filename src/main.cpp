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
#include <WiFiManager.h>
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

#include "modules/telegram/TelegramClient.h"
#include "modules/telegram/TelegramTask.h"
#include "modules/telegram/TelegramCertificate.h"
#include "modules/system/WiFiSetup.h"

#include "modules/time/TimeManager.h"

#define WDT_TIMEOUT 60

static void portalLog(const String& msg) {
    Serial.println(msg);
    addLog(msg);
}

void setup() {
    Serial.begin(115200);
    delay(200);
    // ------------------------------
// FACTORY RESET TRIGGER (Manual Override Switch)
// ------------------------------
pinMode(14, INPUT_PULLUP);   // Manual OPEN
pinMode(27, INPUT_PULLUP);   // Manual CLOSE

// If switch is in CLOSED position at boot, trigger full reset
if (digitalRead(27) == LOW) {
    Serial.println("FACTORY RESET TRIGGERED (override switch CLOSED at boot)");

    // Clear WiFiManager settings
    WiFiManager wm;
    wm.resetSettings();

    // Clear all stored config (Preferences)
    Preferences prefs;
    prefs.begin("config", false);
    prefs.clear();
    prefs.end();

    delay(500);

    // Start WiFi portal
    wm.startConfigPortal("PoultryPortal-Setup");

    // After saving, reboot
    ESP.restart();
}

    portalLog("🐔 PoultryPortal Starting...");

    // --- Hardware Init ---
    HardwarePins_begin();
    Motor_begin();
    DisplayTask_begin();

    // --- Load config FIRST ---
    Config_load();
    addLog("Bot token = [" + Config_getBotToken() + "]");

    // --- WiFi ---
    if (!WiFiSetup_begin()) {
        portalLog("❌ WiFi setup failed — rebooting");
        delay(2000);
        ESP.restart();
    }

    // --- NTP BEFORE ANY TLS ---
    TimeManager::begin();
    portalLog("⏱️ Time synchronized");

    // Wait until Unix time is valid
    time_t now = time(nullptr);
    int retries = 0;
    while (now < 1700000000 && retries < 50) {
        delay(200);
        now = time(nullptr);
        retries++;
    }

    if (now < 1700000000) {
        portalLog("⚠️ Time sync still invalid after waiting");
    } else {
        portalLog("⏱️ Time synchronized (verified)");
    }

    portalLog("Free heap: " + String(ESP.getFreeHeap()));
    portalLog("Free PSRAM: " + String(ESP.getFreePsram()));

    // --- Telegram subsystem ---
    static TelegramClient tgClient(Config_getBotToken(), TELEGRAM_CERTIFICATE_ROOT);
    static TelegramTask tgTask(&tgClient);
    tgTask.start();

    // --- Remaining Subsystems ---
    Battery_begin();
    Energy_begin();
    SensorTask_begin();
    SunContext_begin();
    SchedulerTask_begin();
    AutoMode_begin();

    // OTA init
    ArduinoOTA.setHostname("PoultryPortal");
    ArduinoOTA.begin();

    AutoMode_bootCorrection();
    Serial.println("Bot token: [" + Config_getBotToken() + "]");

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
    delay(200);
    esp_task_wdt_reset();
    ArduinoOTA.handle();
    delay(10);
    
}