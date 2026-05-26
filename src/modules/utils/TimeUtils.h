#pragma once
#include <Arduino.h>
#include <time.h>

// ---------------------------------------------------------
// Compatibility wrapper for legacy code
// All real logic now lives in TimeManager
// ---------------------------------------------------------

// Legacy global sync call → now calls TimeManager::syncNTP()
void TimeUtils_sync();

namespace TimeUtils {

    // Legacy: check if time is valid (NTP synced)
    bool timeIsValid();

    // Legacy: return UTC offset in hours
    int getUTCOffsetHours();

    // Legacy: format timestamp as "YYYY-MM-DD HH:MM"
    String formatTimestamp(time_t t);

    // Legacy: format remaining time (e.g., "1h 20m")
    String formatRemaining(time_t until);

    // Legacy: uptime string (e.g., "1d 3h 22m")
    String getUptime();

    // Legacy: convert minutes-after-midnight to today/tomorrow timestamps
    time_t todayAtMinutes(int minutes);
    time_t tomorrowAtMinutes(int minutes);
}
