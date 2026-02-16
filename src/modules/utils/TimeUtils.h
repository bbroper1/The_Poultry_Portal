#pragma once
#include "modules/time/TimeManager.h"
#include <Arduino.h>
#include <time.h>

// ---------------------------------------------------------
// Compatibility wrapper for legacy code
// All real logic now lives in TimeManager
// ---------------------------------------------------------

// Legacy global sync call → now calls TimeManager::begin()
void TimeUtils_sync();

namespace TimeUtils {

    // Legacy: check if time is valid (NTP synced)
    bool timeIsValid();

    // Legacy: return UTC offset in hours
    int getUTCOffsetHours();

    // Legacy: format timestamp as "YYYY-MM-DD HH:MM"
    String formatTimestamp(time_t t);

    // Legacy: uptime is still handled here (not TZ-related)
    String getUptime();
}