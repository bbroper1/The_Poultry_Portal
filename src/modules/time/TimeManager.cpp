#include "modules/time/TimeManager.h"
#include "modules/config/Config.h"
#include <Arduino.h>
#include <time.h>

namespace TimeManager {

static bool s_timeValid = false;

// ---------------------------------------------------------
// Non-blocking NTP initialization
// ---------------------------------------------------------
void begin() {
    // Apply timezone string
    setenv("TZ", Config_getTimezone().c_str(), 1);

    // Start NTP sync
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    // Do NOT block here — SupervisorTask will monitor validity
    s_timeValid = false;
}

// ---------------------------------------------------------
// Force NTP resync
// ---------------------------------------------------------
void syncNTP() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    s_timeValid = false;
}

// ---------------------------------------------------------
// Check if time is valid
// ---------------------------------------------------------
bool isValid() {
    time_t now = time(nullptr);

    if (now > 1700000000) {   // ~2023+
        if (!s_timeValid) {
            tzset();          // Apply timezone AFTER valid time
            s_timeValid = true;
        }
        return true;
    }

    return false;
}

// ---------------------------------------------------------
// UTC offset in hours
// ---------------------------------------------------------
int utcOffsetHours() {
    time_t now = time(nullptr);

    struct tm localTm = *localtime(&now);
    struct tm gmTm    = *gmtime(&now);

    int offset = localTm.tm_hour - gmTm.tm_hour;

    if (localTm.tm_yday > gmTm.tm_yday) offset += 24;
    if (localTm.tm_yday < gmTm.tm_yday) offset -= 24;

    return offset;
}

// ---------------------------------------------------------
// Local time struct
// ---------------------------------------------------------
struct tm getLocalTime() {
    time_t now = time(nullptr);
    return *localtime(&now);
}

// ---------------------------------------------------------
// Format timestamp (no seconds)
// ---------------------------------------------------------
String formatTimestamp(time_t t) {
    if (t <= 0) return "N/A";

    struct tm tmInfo = *localtime(&t);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tmInfo);
    return String(buf);
}

// ---------------------------------------------------------
// Format timestamp with seconds
// ---------------------------------------------------------
String formatTimestampSeconds(time_t t) {
    if (t <= 0) return "N/A";

    struct tm tmInfo = *localtime(&t);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmInfo);
    return String(buf);
}

// ---------------------------------------------------------
// Monotonic uptime in seconds
// ---------------------------------------------------------
unsigned long uptimeSeconds() {
    return millis() / 1000;
}

} // namespace TimeManager
