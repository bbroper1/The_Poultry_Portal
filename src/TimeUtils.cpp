#include "TimeUtils.h"
#include "Config.h"
#include "Logging.h"

#include <Arduino.h>
#include <time.h>

void TimeUtils_sync() {
    String tz = Config_getTimezone();
    Serial.println("Applying timezone: " + tz);

    configTzTime(tz.c_str(), "pool.ntp.org", "time.nist.gov");

    time_t now = time(nullptr);
    int retries = 0;

    while (now < 100000 && retries < 50) {
        delay(100);
        now = time(nullptr);
        retries++;
    }

    if (now >= 100000) {
        Serial.println("⏱️ Time synchronized");
        addLog("Time synchronized ⏱️");
    } else {
        Serial.println("⚠️ Time sync failed or slow");
        addLog("Time sync failed ⚠️");
    }
}

int TimeUtils_getUTCOffsetHours() {
    time_t now = time(nullptr);
    struct tm localTm = *localtime(&now);
    struct tm gmTm    = *gmtime(&now);

    // Difference in hours
    int offset = localTm.tm_hour - gmTm.tm_hour;

    // Adjust for day rollover
    if (localTm.tm_yday > gmTm.tm_yday) offset += 24;
    if (localTm.tm_yday < gmTm.tm_yday) offset -= 24;

    return offset;
}

bool TimeUtils_timeIsValid() {
    time_t now = time(nullptr);
    return now > 100000;   // same logic you used before
}

String TimeUtils_getUptime() {
    unsigned long secs = millis() / 1000;
    unsigned long mins = secs / 60;
    unsigned long hours = mins / 60;

    char buf[32];
    sprintf(buf, "%luh %lum", hours, mins % 60);
    return String(buf);
}
