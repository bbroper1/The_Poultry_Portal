#include "BatteryModule.h"
#include <Adafruit_INA219.h>
#include <Arduino.h>
#include "modules/system/Logging.h"

// ---------------------------------------------------------
// INA219 INSTANCE
// ---------------------------------------------------------
static Adafruit_INA219 ina219;

// ---------------------------------------------------------
// Cached values
// ---------------------------------------------------------
static float s_voltageRaw       = 12.4f;
static float s_voltageEMA       = 12.4f;
static float s_voltageAlpha     = 0.10f;   // smoothing factor

static float s_current_mA       = 0.0f;

static bool  s_inaOK            = false;

// Daily min/max
static float s_battMinToday     = 999.0f;
static float s_battMaxToday     = 0.0f;
static int   s_lastDay          = -1;

// Brownout threshold
static const float CRITICAL_VOLTAGE = 10.5f;

// ---------------------------------------------------------
// Daily min/max helper
// ---------------------------------------------------------
static void updateDailyMinMax(float v) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    if (t->tm_mday != s_lastDay) {
        s_lastDay = t->tm_mday;
        s_battMinToday = 999.0f;
        s_battMaxToday = 0.0f;
    }

    if (v < s_battMinToday) s_battMinToday = v;
    if (v > s_battMaxToday) s_battMaxToday = v;
}

float Battery_getMinToday() { return s_battMinToday; }
float Battery_getMaxToday() { return s_battMaxToday; }

// ---------------------------------------------------------
// INITIALIZATION
// ---------------------------------------------------------
void Battery_begin() {
    Serial.println("[Battery] Initializing INA219...");

    if (!ina219.begin()) {
        Serial.println("[Battery] ERROR: INA219 not detected!");
        s_inaOK = false;
        return;
    }

    ina219.setCalibration_16V_400mA();
    s_inaOK = true;

    Serial.println("[Battery] INA219 initialized OK");
}

// ---------------------------------------------------------
// UPDATE FROM INA219 (called by SensorTask once per second)
// ---------------------------------------------------------
void Battery_updateFromINA219() {
    if (!s_inaOK) return;

    float busV = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();

    if (isnan(busV) || isnan(current_mA)) {
        s_inaOK = false;
        addLog("Battery → INA219 returned NaN");
        return;
    }

    s_voltageRaw = busV;
    s_current_mA = current_mA;

    // EMA smoothing
    s_voltageEMA = (s_voltageAlpha * busV) + ((1.0f - s_voltageAlpha) * s_voltageEMA);

    updateDailyMinMax(busV);
}

// ---------------------------------------------------------
// Setters (override only)
// ---------------------------------------------------------
void Battery_setVoltage(float v) {
    s_voltageRaw = v;
    s_voltageEMA = v;
}

void Battery_setCurrentmA(float mA) {
    s_current_mA = mA;
}

// ---------------------------------------------------------
// Getters
// ---------------------------------------------------------
float Battery_getVoltage() {
    return s_voltageRaw;
}

float Battery_getSmoothedVoltage() {
    return s_voltageEMA;
}

float Battery_getCurrentmA() {
    return s_current_mA;
}

// ---------------------------------------------------------
// Convert voltage → percent (simple curve)
// ---------------------------------------------------------
int Battery_getPercent() {
    float v = s_voltageRaw;

    if (v >= 12.6f) return 100;
    if (v >= 12.4f) return 90;
    if (v >= 12.2f) return 80;
    if (v >= 12.0f) return 70;
    if (v >= 11.8f) return 60;
    if (v >= 11.6f) return 50;
    if (v >= 11.4f) return 40;
    if (v >= 11.2f) return 30;
    if (v >= 11.0f) return 20;
    return 10;
}

// ---------------------------------------------------------
// Health helpers
// ---------------------------------------------------------
bool Battery_isValid() {
    return s_inaOK;
}

bool Battery_isCritical() {
    return s_voltageRaw < CRITICAL_VOLTAGE;
}

bool Battery_isBrownout() {
    return s_voltageEMA < CRITICAL_VOLTAGE;
}

// ---------------------------------------------------------
// Smoothing control
// ---------------------------------------------------------
void Battery_setVoltageSmoothing(float alpha) {
    s_voltageAlpha = constrain(alpha, 0.01f, 1.0f);
}

// ---------------------------------------------------------
// Optional: INA219 temperature (if supported)
// ---------------------------------------------------------
float Battery_readTemperatureC() {
    // INA219 does NOT provide temperature.
    // Replace with your actual sensor if needed.
    return NAN;
}
