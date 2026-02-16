#pragma once
#include <Arduino.h>
#include <time.h>

namespace TimeManager {

    // Call once at boot (after WiFi connects)
    void begin();

    // Returns true if NTP time is valid
    bool isValid();

    // Returns local UTC offset in hours (e.g., -6 for CST)
    int utcOffsetHours();

    // Returns a fully-populated localtime struct
    struct tm getLocalTime();

    // Formats a timestamp into "YYYY-MM-DD HH:MM"
    String formatTimestamp(time_t t);
}