#include "modules/sensors/SensorModule.h"

// Cached temperature (default until first update)
static float s_currentTempC = 21.3f;

// ---------------------------------------------------------
// Set temperature (called by SensorTask)
// ---------------------------------------------------------
void Sensor_setTemperature(float celsius) {
    s_currentTempC = celsius;
}

// ---------------------------------------------------------
// Get temperature
// ---------------------------------------------------------
float Sensor_getTemperature() {
    return s_currentTempC;
}