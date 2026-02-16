#include "modules/time/TimeManager.h"
#include "modules/config/Config.h"
#include <Arduino.h>
#include <time.h>

namespace TimeManager {

void begin() {
    // 1. Set timezone string
    setenv("TZ", Config_getTimezone().c_str(), 1);

    // 2. Start NTP sync
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    // 3. Wait for valid time
    time_t now = time(nullptr);
    int retries = 0;
    while (now < 1700000000 && retries < 50) {
        delay(200);
        now = time(nullptr);
        retries++;
    }

    // 4. Apply timezone AFTER time is valid
    tzset();

    Serial.printf("TimeManager: final offset = %d\n", TimeManager::utcOffsetHours());
}

bool isValid() {
    time_t now = time(nullptr);
    return now > 100000;   // or your preferred threshold
}

int utcOffsetHours() {
    time_t now = time(nullptr);

    struct tm localTm = *localtime(&now);
    struct tm gmTm    = *gmtime(&now);

    int offset = localTm.tm_hour - gmTm.tm_hour;

    if (localTm.tm_yday > gmTm.tm_yday) offset += 24;
    if (localTm.tm_yday < gmTm.tm_yday) offset -= 24;

    return offset;
}

struct tm getLocalTime() {
    time_t now = time(nullptr);
    struct tm t = *localtime(&now);
    return t;
}

String formatTimestamp(time_t t) {
    if (t <= 0) return "N/A";

    struct tm tmInfo = *localtime(&t);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tmInfo);
    return String(buf);
}

} // namespace TimeManager  