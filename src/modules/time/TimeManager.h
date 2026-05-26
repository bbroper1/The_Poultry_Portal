#pragma once
#include <Arduino.h>
#include <time.h>

namespace TimeManager {

    // Initialize NTP + timezone (non-blocking)
    void begin();

    // Force NTP resync (safe to call anytime)
    void syncNTP();

    // Returns true if NTP time is valid
    bool isValid();

    // Returns local UTC offset in hours (e.g., -6 for CST)
    int utcOffsetHours();

    // Returns a fully-populated localtime struct
    struct tm getLocalTime();

    // Formats a timestamp into "YYYY-MM-DD HH:MM"
    String formatTimestamp(time_t t);

    // Formats with seconds "YYYY-MM-DD HH:MM:SS"
    String formatTimestampSeconds(time_t t);

    // Returns millis()/1000 as a monotonic uptime timestamp
    unsigned long uptimeSeconds();
}
