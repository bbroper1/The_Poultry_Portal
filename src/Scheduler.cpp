#include "Scheduler.h"
#include "Config.h"
#include "Globals.h"
#include "TimeUtils.h"
#include <sunset.h>
#include <time.h>

extern SunSet sun;

void Scheduler_init() {}

void Scheduler_calcLocalSunTimes(int &sunriseLocal, int &sunsetLocal) {
    float lat = Config_getLat();
    float lon = Config_getLong();
    float tzHours = TimeUtils_getUTCOffsetHours();

    sun.setPosition(lat, lon, tzHours);

    int sunrise = (int)sun.calcSunrise();
    int sunset  = (int)sun.calcSunset();

    sunriseLocal = sunrise + Config_getOpenOffset();
    sunsetLocal  = sunset  + Config_getCloseOffset();

    if (sunriseLocal < 0) sunriseLocal += 1440;
    if (sunriseLocal >= 1440) sunriseLocal -= 1440;

    if (sunsetLocal < 0) sunsetLocal += 1440;
    if (sunsetLocal >= 1440) sunsetLocal -= 1440;
}

static String formatSmartTime(int minutes, int offset, const struct tm* date) {
    char buf[40];
    sprintf(buf, "%02d/%02d: %d:%02d (+%d)",
            date->tm_mon + 1,
            date->tm_mday,
            minutes / 60,
            minutes % 60,
            offset);
    return String(buf);
}

String Scheduler_getNextOpen() {
    if (!TimeUtils_timeIsValid()) return "--:--";

    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);

    int sunriseLocal, sunsetLocal;
    Scheduler_calcLocalSunTimes(sunriseLocal, sunsetLocal);

    int minutesNow = ptm->tm_hour * 60 + ptm->tm_min;

    if (minutesNow < sunriseLocal)
        return formatSmartTime(sunriseLocal, Config_getOpenOffset(), ptm);

    struct tm tomorrow = *ptm;
    tomorrow.tm_mday += 1;
    mktime(&tomorrow);

    sun.setPosition(Config_getLat(), Config_getLong(), TimeUtils_getUTCOffsetHours()
);
    sun.setCurrentDate(tomorrow.tm_year + 1900,
                       tomorrow.tm_mon + 1,
                       tomorrow.tm_mday);

    int sr = (int)sun.calcSunrise() + Config_getOpenOffset();
    if (sr < 0) sr += 1440;
    if (sr >= 1440) sr -= 1440;

    return formatSmartTime(sr, Config_getOpenOffset(), &tomorrow);
}

String Scheduler_getNextClose() {
    if (!TimeUtils_timeIsValid()) return "--:--";

    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);

    int sunriseLocal, sunsetLocal;
    Scheduler_calcLocalSunTimes(sunriseLocal, sunsetLocal);

    int minutesNow = ptm->tm_hour * 60 + ptm->tm_min;

    if (minutesNow < sunsetLocal)
        return formatSmartTime(sunsetLocal, Config_getCloseOffset(), ptm);

    struct tm tomorrow = *ptm;
    tomorrow.tm_mday += 1;
    mktime(&tomorrow);

    sun.setPosition(Config_getLat(), Config_getLong(), TimeUtils_getUTCOffsetHours());
    sun.setCurrentDate(tomorrow.tm_year + 1900,
                       tomorrow.tm_mon + 1,
                       tomorrow.tm_mday);

    int ss = (int)sun.calcSunset() + Config_getCloseOffset();
    if (ss < 0) ss += 1440;
    if (ss >= 1440) ss -= 1440;

    return formatSmartTime(ss, Config_getCloseOffset(), &tomorrow);
}
