    #include "modules/utils/TimeUtils.h"
    #include "modules/time/TimeManager.h"
    #include <Arduino.h>

    // ---------------------------------------------------------
    // Compatibility layer for legacy TimeUtils API
    // All real logic now lives in TimeManager
    // ---------------------------------------------------------

    // Legacy global sync call → now calls TimeManager::begin()
    void TimeUtils_sync() {
        TimeManager::begin();
    }

    namespace TimeUtils {

    bool timeIsValid() {
        return TimeManager::isValid();
    }

    int getUTCOffsetHours() {
        return TimeManager::utcOffsetHours();
    }

    String formatTimestamp(time_t t) {
        return TimeManager::formatTimestamp(t);
    }

    // Uptime is not timezone-related, so it stays here
    String getUptime() {
        unsigned long secs = millis() / 1000;

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