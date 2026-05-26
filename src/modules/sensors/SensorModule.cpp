#include "modules/sensors/SensorModule.h"
#include "modules/system/Logging.h"

// ---------------------------------------------------------
// Internal state
// ---------------------------------------------------------
static float s_tempC = 21.3f;      // smoothed temperature
static float s_alpha = 0.15f;      // EMA smoothing factor
static time_t s_lastUpdate = 0;    // millis timestamp

// ---------------------------------------------------------
// Initialize
// ---------------------------------------------------------
void Sensor_begin() {
    s_tempC = 21.3f;
    s_lastUpdate = 0;
    s_alpha = 0.15f;
}

// ---------------------------------------------------------
// Configure smoothing
// ---------------------------------------------------------
void Sensor_setSmoothing(float alpha) {
    s_alpha = constrain(alpha, 0.01f, 1.0f);
}

// ---------------------------------------------------------
// Set temperature (EMA smoothed)
// ---------------------------------------------------------
void Sensor_setTemperature(float celsius) {
    if (!isnan(celsius)) {
        // EMA smoothing: new = α*value + (1-α)*old
        s_tempC = (s_alpha * celsius) + ((1.0f - s_alpha) * s_tempC);
        s_lastUpdate = millis();
    } else {
        addLog("SensorModule → Invalid temperature (NaN)");
    }
}

// ---------------------------------------------------------
// Get temperature
// ---------------------------------------------------------
float Sensor_getTemperature() {
    return s_tempC;
}

int Sensor_getTemperatureInt() {
    return (int)s_tempC;
}

// ---------------------------------------------------------
// Timestamp of last update
// ---------------------------------------------------------
time_t Sensor_getLastUpdateTime() {
    return s_lastUpdate;
}

// ---------------------------------------------------------
// Freshness check
// ---------------------------------------------------------
bool Sensor_isFresh(unsigned long maxAgeMs) {
    if (s_lastUpdate == 0) return false;
    return (millis() - s_lastUpdate) <= maxAgeMs;
}
