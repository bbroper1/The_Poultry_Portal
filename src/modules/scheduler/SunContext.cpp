#include "modules/scheduler/SunContext.h"
#include "modules/config/Config.h"
#include "modules/time/TimeManager.h"
#include "modules/system/Logging.h"

// ---------------------------------------------------------
// Global SunSet instance
// ---------------------------------------------------------
SunSet sun;

// ---------------------------------------------------------
// Internal helper
// ---------------------------------------------------------
static void applySunPosition() {
    float lat     = Config_getLat();
    float lon     = Config_getLong();
    float tzHours = TimeManager::utcOffsetHours();

    sun.setPosition(lat, lon, tzHours);
}

// ---------------------------------------------------------
// Initialize SunSet with latitude, longitude, timezone
// ---------------------------------------------------------
void SunContext_begin() {
    if (!TimeManager::isValid()) {
        addLog("SunContext → Time invalid at init, using tz=0");
    }

    applySunPosition();
    addLog("SunContext → Initialized");
}

// ---------------------------------------------------------
// Refresh after config/timezone changes
// ---------------------------------------------------------
void SunContext_refresh() {
    applySunPosition();
    addLog("SunContext → Refreshed");
}
