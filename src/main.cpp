/************************************************************
 * POULTRY PORTAL — MAIN FIRMWARE (WEB-SERIAL VERSION)
 ************************************************************/

#include <WiFi.h>
#include <WiFiClientSecure.h>
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
#include <nvs.h>
#include <nvs_flash.h>

// --- NEW WIRELESS DEBUG LIBRARIES ---
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

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
SunSet sun;
Preferences prefs;
AsyncWebServer server(80); // Create AsyncWebServer on port 80

// --- WIRELESS LOGGING HELPER ---
void portalLog(String msg) {
    Serial.println(msg);
    WebSerial.println(msg);
}

// -------------------------------
//  SETUP
// -------------------------------
void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n\n🐔 PoultryPortal Starting...");
    WebSerial.println("\n\n🐔 PoultryPortal Starting...");
    bootTime = millis();

    // ---------------------------------------------------------
    //  FACTORY RESET & HARDWARE INIT (Same as your version)
    // ---------------------------------------------------------
    pinMode(14, INPUT_PULLUP);
    pinMode(27, INPUT_PULLUP);
    bool closeOverride = (digitalRead(14) == LOW);

    if (closeOverride) {
        factoryResetRequested = true;
        // ... (Preferences clear logic remains the same)
    }

    HardwarePins_init();
    Temperature_begin();
    Motor_begin();
    Display_begin();
    Config_load();

    // ---------------------------------------------------------
    //  WIFI & WEB SERIAL INIT
    // ---------------------------------------------------------
    if (!WiFiSetup_begin()) {
        Serial.println("❌ WiFi failed — rebooting");
        delay(2000);
        ESP.restart();
    }
    
    // Start WebSerial after WiFi is connected
    WebSerial.begin(&server);
    server.begin();
    
    portalLog("✅ WiFi connected");
    portalLog("🌐 WebSerial active at http://" + WiFi.localIP().toString() + "/webserial");

    // ---------------------------------------------------------
    //  I2C / NTP / OTHER MODULES
    // ---------------------------------------------------------
    Wire.begin(21, 22);
    Wire.setClock(400000);
    inaOK = ina219.begin(&Wire);

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Battery_begin();
    TimeUtils_sync();
    Energy_begin();
    
    secured_client.setInsecure();
    TelegramRouter_init();

    ArduinoOTA.setHostname("PoultryPortal");
    ArduinoOTA.begin();

    addLog("Online 🚀");
    portalLog("=== BOOT COMPLETE ===");
    AutoMode_bootCorrection();
}

void loop() {
    esp_task_wdt_reset();
    ArduinoOTA.handle();

    if (telegramEnabled) {
        TelegramRouter_handle();
    }

    Motor_update();
    Temperature_update();
    Battery_update();
    Energy_update();
    Display_update();
    SystemStatus_updateHeartbeat();
    yield(); // Give system tasks a moment to process WebSerial
    delay(1);
    // Note: WebSerial handles itself via the AsyncServer background task
}