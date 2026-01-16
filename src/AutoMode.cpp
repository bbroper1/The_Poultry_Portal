#include "AutoMode.h"
#include "Globals.h"
#include "Config.h"
#include "Motor.h"
#include "Scheduler.h"
#include "Logging.h"
#include "TimeUtils.h"
#include "HardwarePins.h"
#include <sunset.h>
#include <time.h>

extern SunSet sun;

void AutoMode_bootCorrection() {
    if (remoteOverride) {
        addLog("Auto Boot → Skipped (manual override)");
        return;
    }

    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);

    if (!ptm) {
        addLog("Auto Boot → Time invalid, skipping");
        return;
    }

    // Configure sun position
    sun.setCurrentDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
    sun.setPosition(Config_getLat(), Config_getLong(), TimeUtils_getUTCOffsetHours());

    int sunrise = (int)sun.calcSunrise() + Config_getOpenOffset();
    int sunset  = (int)sun.calcSunset()  + Config_getCloseOffset();
    int minutesNow = ptm->tm_hour * 60 + ptm->tm_min;

    bool shouldBeOpen = (minutesNow >= sunrise && minutesNow < sunset);

    // ---------------------------------------------------------
    //  CORRECTION LOGIC
    // ---------------------------------------------------------
    if (shouldBeOpen) {
        if (digitalRead(PIN_LIMIT_OPEN) == HIGH) {
            Motor_requestOpen();
            addLog("Auto Boot → Opening (should be open)");
        } else {
            addLog("Auto Boot → Already open");
        }
    } else {
        if (digitalRead(PIN_LIMIT_CLOSE) == HIGH) {
            Motor_requestClose();
            addLog("Auto Boot → Closing (should be closed)");
        } else {
            addLog("Auto Boot → Already closed");
        }
    }
}
