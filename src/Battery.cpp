#include "Battery.h"
#include "Energy.h"
#include "Globals.h"
#include "Config.h"
#include "Logging.h"
#include <Adafruit_INA219.h>
#include <Preferences.h>
#include <time.h>
#include <Wire.h>

extern Adafruit_INA219 ina219;
extern Preferences prefs;

static const char* NAMESPACE = "battery";
static bool s_lowBatAlertSent = false;

// Cached readings
static float s_lastVoltage = 0.0f;
static float s_lastCurrent = 0.0f;

// Thresholds (module‑private)
static constexpr float BATTERY_WARNING_VOLTAGE  = 11.5;
static constexpr float BATTERY_CRITICAL_VOLTAGE = 10.5;

// ---------------------------------------------------------
//  GETTERS
// ---------------------------------------------------------
float Battery_getWarningVoltage() { return BATTERY_WARNING_VOLTAGE; }
float Battery_getCriticalVoltage() { return BATTERY_CRITICAL_VOLTAGE; }

// ---------------------------------------------------------
//  BEGIN
// ---------------------------------------------------------
void Battery_begin() {

    // INA219 is initialized in setup(), not here.
    // We only load prefs + daily reset logic.

    prefs.begin(NAMESPACE, false);

    float savedToday = prefs.getFloat("todaymAh", 0.0f);
    time_t lastReset = prefs.getULong("lastReset", 0);
    s_lowBatAlertSent = prefs.getBool("lowAlert", false);

    // Daily reset logic
    time_t now = time(nullptr);
    struct tm* nowTm  = localtime(&now);
    struct tm* lastTm = localtime(&lastReset);

    bool newDay = false;

    if (lastReset == 0) {
        newDay = true;
    } else if (nowTm && lastTm) {
        if (nowTm->tm_year != lastTm->tm_year ||
            nowTm->tm_yday != lastTm->tm_yday) {
            newDay = true;
        }
    }

    if (newDay) {
        prefs.putFloat("todaymAh", 0.0f);
        prefs.putULong("lastReset", now);
        savedToday = 0.0f;
    }

    // Push into Energy module
    Energy_setTodaymAh(savedToday);
    Energy_setLastReset(lastReset);

    prefs.end();
}

// ---------------------------------------------------------
//  UPDATE
// ---------------------------------------------------------
void Battery_update() {

    if (!inaOK) {
        s_lastVoltage = 0.0f;
        s_lastCurrent = 0.0f;
        return;
    }

    s_lastVoltage = ina219.getBusVoltage_V();
    s_lastCurrent = ina219.getCurrent_mA();
}

// ---------------------------------------------------------
//  STATE QUERIES
// ---------------------------------------------------------
float Battery_getVoltage() { return s_lastVoltage; }
float Battery_getCurrent() { return s_lastCurrent; }

bool Battery_isLow()      { return s_lastVoltage < BATTERY_WARNING_VOLTAGE; }
bool Battery_isCritical() { return s_lastVoltage < BATTERY_CRITICAL_VOLTAGE; }

bool Battery_alertSent()  { return s_lowBatAlertSent; }
