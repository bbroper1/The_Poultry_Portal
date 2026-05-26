#include "modules/utils/TimeUtils.h"
#include "modules/time/TimeManager.h"
#include <Arduino.h>

void TimeUtils_sync() {
    // Legacy API → modern behavior
    TimeManager::syncNTP();
}

namespace TimeUtils {

// ---------------------------------------------------------
// Legacy: is time valid?
// ---------------------------------------------------------
bool timeIsValid() {
    return TimeManager::isValid();
}

// ---------------------------------------------------------
// Legacy: UTC offset
// ---------------------------------------------------------
int getUTCOffsetHours() {
    return TimeManager::utcOffsetHours();
}

// ---------------------------------------------------------
// Legacy: format timestamp
// ---------------------------------------------------------
String formatTimestamp(time_t t) {
    return TimeManager::formatTimestamp(t);
}

// ---------------------------------------------------------
// Legacy: format remaining time
// ---------------------------------------------------------
String formatRemaining(time_t until) {
    time_t now = time(nullptr);
    if (until <= now) return "0m";

    int sec = until - now;
    int min = sec / 60;
    int hr  = min / 60;

    if (hr > 0)
        return String(hr) + "h " + String(min % 60) + "m";

    return String(min) + "m";
}

// ---------------------------------------------------------
// Legacy: today at minutes
// ---------------------------------------------------------
time_t todayAtMinutes(int minutes) {
    struct tm nowTm = TimeManager::getLocalTime();
    nowTm.tm_hour = minutes / 60;
    nowTm.tm_min  = minutes % 60;
    nowTm.tm_sec  = 0;
    nowTm.tm_isdst = -1;
    return mktime(&nowTm);
}

// ---------------------------------------------------------
// Legacy: tomorrow at minutes
// ---------------------------------------------------------
time_t tomorrowAtMinutes(int minutes) {
    struct tm nowTm = TimeManager::getLocalTime();
    nowTm.tm_mday += 1;
    nowTm.tm_hour = minutes / 60;
    nowTm.tm_min  = minutes % 60;
    nowTm.tm_sec  = 0;
    nowTm.tm_isdst = -1;
    return mktime(&nowTm);
}

// ---------------------------------------------------------
// Legacy: uptime string
// ---------------------------------------------------------
String getUptime() {
    unsigned long secs = TimeManager::uptimeSeconds();

    unsigned long mins  = secs / 60;
    unsigned long hours = mins / 60;
    unsigned long days  = hours / 24;
    unsigned long months = days / 30;   // simple 30‑day month
    days %= 30;
    hours %= 24;
    mins %= 60;

    String out = "";

    if (months > 0)
        out += String(months) + "mo ";

    if (days > 0)
        out += String(days) + "d ";

    out += String(hours) + "h ";
    out += String(mins) + "m";

    return out;
}

} // namespace TimeUtils
