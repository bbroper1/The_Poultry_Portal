#include "modules/scheduler/SunContext.h"
#include "modules/config/Config.h"
#include "modules/time/TimeManager.h"

// ---------------------------------------------------------
// Global SunSet instance
// ---------------------------------------------------------
SunSet sun;

// ---------------------------------------------------------
// Initialize SunSet with latitude, longitude, timezone
// ---------------------------------------------------------
void SunContext_begin() {
    float lat     = Config_getLat();
    float lon     = Config_getLong();
    float tzHours = TimeManager::utcOffsetHours();   // ✅ correct API

    sun.setPosition(lat, lon, tzHours);
}