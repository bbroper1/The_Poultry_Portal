/************************************************************
 *  POULTRY PORTAL — MAIN FIRMWARE (CLEAN VERSION)
 ************************************************************/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AsyncTelegram2.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include <sunset.h>
#include <Preferences.h>
#include <WiFiManager.h>
#include <esp_task_wdt.h>

// -------------------------------
//  MODULES
// -------------------------------
#include "Globals.h"
#include "Config.h"
#include "Scheduler.h"
#include "TelegramRouter.h"
#include "Logging.h"
#include "Display.h"
#include "Temperature.h"
#include "SystemStatus.h"
#include "TimeUtils.h"
#include "HardwarePins.h"
#include "Energy.h"
#include "Battery.h"
#include "Motor.h"
#include "AutoMode.h"
#include "WiFiSetup.h"

#define WDT_TIMEOUT 60

// -------------------------------
//  GLOBAL OBJECTS
// -------------------------------
Adafruit_INA219 ina219;
WiFiClientSecure secured_client;
AsyncTelegram2 bot(secured_client);
SunSet sun;
Preferences prefs;

// -------------------------------
//  SETUP
// -------------------------------
void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n\n🐔 PoultryPortal Starting...");

    bootTime = millis();

    // --- CORE HARDWARE ---
    HardwarePins_init();
    Temperature_begin();
    Motor_begin();
    Display_begin();

    // --- LOAD CONFIG ---
    Config_load();

    // --- WIFI ---
    if (!WiFiSetup_begin()) {
        Serial.println("❌ WiFi failed — rebooting");
        delay(2000);
        ESP.restart();
    }
    Serial.println("✅ WiFi connected");

    // --- I2C BUS ---
    Wire.begin(21, 22);
    Wire.setClock(400000);

    // --- BATTERY / INA219 ---
    Battery_begin();

    // --- TIME SYNC ---
    TimeUtils_sync();

    // --- TELEGRAM ---
    secured_client.setInsecure();
    TelegramRouter_init();

    // --- OTA ---
    ArduinoOTA.setHostname("PoultryPortal");
    if (Config_getOTAPassword().length() > 0) {
        ArduinoOTA.setPassword(Config_getOTAPassword().c_str());
    }
    ArduinoOTA.begin();

    // --- ENERGY TRACKING ---
    Energy_begin();

    addLog("Online 🚀");
    Serial.println("=== BOOT COMPLETE ===\n");

    // --- AUTO MODE BOOT CORRECTION ---
    AutoMode_bootCorrection();

    bootTime = millis();
}

// -------------------------------
//  LOOP
// -------------------------------
void loop() {
    esp_task_wdt_reset();

    ArduinoOTA.handle();
    TelegramRouter_handle();

    Motor_update();
    Temperature_update();
    Battery_update();
    Energy_update();
    Display_update();

    SystemStatus_updateHeartbeat();
}
