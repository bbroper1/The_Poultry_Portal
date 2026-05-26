#include "modules/scheduler/SchedulerTask.h"

#include "modules/config/Config.h"
#include "modules/system/Globals.h"
#include "modules/time/TimeManager.h"
#include "modules/motor/MotorTask.h"
#include "modules/system/Logging.h"
#include "modules/scheduler/SunContext.h"

#include <sunset.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t s_schedulerTaskHandle = nullptr;

// ---------------------------------------------------------
// External SunSet instance (owned by SunContext)
// ---------------------------------------------------------
extern SunSet sun;
extern bool remoteOverride;

// ---------------------------------------------------------
// Internal context
// ---------------------------------------------------------
struct SchedulerContext {
    TickType_t lastWake = 0;
};

static SchedulerContext* ctx = nullptr;

// ---------------------------------------------------------
// Utility: wrap minutes into 0–1439
// ---------------------------------------------------------
static inline int wrapMinutes(int m) {
    if (m < 0)     return m + 1440;
    if (m >= 1440) return m - 1440;
    return m;
}

// ---------------------------------------------------------
// RAW Sunrise / Sunset Calculation (NO OFFSETS)
// ---------------------------------------------------------
void Scheduler_calcLocalSunTimes(int &sunriseLocal, int &sunsetLocal) {

    float lat = Config_getLat();
    float lon = Config_getLong();
    float tzHours = TimeManager::utcOffsetHours();

    sun.setPosition(lat, lon, tzHours);

    struct tm nowTm = TimeManager::getLocalTime();
    sun.setCurrentDate(
        nowTm.tm_year + 1900,
        nowTm.tm_mon + 1,
        nowTm.tm_mday
    );

    sunriseLocal = wrapMinutes((int)sun.calcSunrise());
    sunsetLocal  = wrapMinutes((int)sun.calcSunset());
}

// ---------------------------------------------------------
// Format Smart Time (final scheduled time)
// ---------------------------------------------------------
static String formatSmartTime(int minutes, int offset, const struct tm* date) {
    char buf[40];
    sprintf(buf, "%02d/%02d: %02d:%02d (%+d)",
            date->tm_mon + 1,
            date->tm_mday,
            minutes / 60,
            minutes % 60,
            offset);
    return String(buf);
}

// ---------------------------------------------------------
// Helper: Should door be open right now?
// ---------------------------------------------------------
bool Scheduler_shouldBeOpenNow() {
    if (!TimeManager::isValid())
        return false;

    int sunriseRaw, sunsetRaw;
    Scheduler_calcLocalSunTimes(sunriseRaw, sunsetRaw);

    int openOffset  = Config_getOpenOffset();
    int closeOffset = Config_getCloseOffset();

    int sunriseAdj = wrapMinutes(sunriseRaw + openOffset);
    int sunsetAdj  = wrapMinutes(sunsetRaw + closeOffset);

    struct tm nowTm = TimeManager::getLocalTime();
    int minutesNow = nowTm.tm_hour * 60 + nowTm.tm_min;

    return (minutesNow >= sunriseAdj && minutesNow < sunsetAdj);
}

// ---------------------------------------------------------
// NEXT SMART OPEN
// ---------------------------------------------------------
String Scheduler_getNextOpen() {
    if (!TimeManager::isValid()) return "--:--";

    struct tm nowTm = TimeManager::getLocalTime();
    int sunriseRaw, sunsetRaw;
    Scheduler_calcLocalSunTimes(sunriseRaw, sunsetRaw);

    int offset = Config_getOpenOffset();
    int sunriseAdj = wrapMinutes(sunriseRaw + offset);

    int minutesNow = nowTm.tm_hour * 60 + nowTm.tm_min;

    // Today
    if (minutesNow < sunriseAdj)
        return formatSmartTime(sunriseAdj, offset, &nowTm);

    // Tomorrow
    struct tm tomorrow = nowTm;
    tomorrow.tm_mday += 1;
    mktime(&tomorrow);

    sun.setPosition(Config_getLat(), Config_getLong(), TimeManager::utcOffsetHours());
    sun.setCurrentDate(
        tomorrow.tm_year + 1900,
        tomorrow.tm_mon + 1,
        tomorrow.tm_mday
    );

    int srRaw = (int)sun.calcSunrise();
    int sr = wrapMinutes(srRaw + offset);

    return formatSmartTime(sr, offset, &tomorrow);
}

// ---------------------------------------------------------
// NEXT SMART CLOSE
// ---------------------------------------------------------
String Scheduler_getNextClose() {
    if (!TimeManager::isValid()) return "--:--";

    struct tm nowTm = TimeManager::getLocalTime();
    int sunriseRaw, sunsetRaw;
    Scheduler_calcLocalSunTimes(sunriseRaw, sunsetRaw);

    int offset = Config_getCloseOffset();
    int sunsetAdj = wrapMinutes(sunsetRaw + offset);

    int minutesNow = nowTm.tm_hour * 60 + nowTm.tm_min;

    // Today
    if (minutesNow < sunsetAdj)
        return formatSmartTime(sunsetAdj, offset, &nowTm);

    // Tomorrow
    struct tm tomorrow = nowTm;
    tomorrow.tm_mday += 1;
    mktime(&tomorrow);

    sun.setPosition(Config_getLat(), Config_getLong(), TimeManager::utcOffsetHours());
    sun.setCurrentDate(
        tomorrow.tm_year + 1900,
        tomorrow.tm_mon + 1,
        tomorrow.tm_mday
    );

    int ssRaw = (int)sun.calcSunset();
    int ss = wrapMinutes(ssRaw + offset);

    return formatSmartTime(ss, offset, &tomorrow);
}

// ---------------------------------------------------------
// FreeRTOS Scheduler Task
// ---------------------------------------------------------
static void SchedulerTask(void* pvParameters) {
    setenv("TZ", Config_getTimezone().c_str(), 1);
    tzset();

    SchedulerContext* c = ctx;
    const TickType_t interval = pdMS_TO_TICKS(60000);
    c->lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&c->lastWake, interval);

        if (!TimeManager::isValid()) {
            addLog("Scheduler → Time invalid, skipping");
            continue;
        }

        if (remoteOverride) {
            addLog("Scheduler → Override active, skipping");
            continue;
        }

        bool shouldBeOpen = Scheduler_shouldBeOpenNow();
        MotorDoorState state = Motor_getState();

        if (shouldBeOpen) {
            if (state != M_OPEN && state != M_OPENING) {
                Motor_requestOpen();
                addLog("Scheduler → Opening (scheduled)");
            }
        } else {
            if (state != M_CLOSED && state != M_CLOSING) {
                Motor_requestClose();
                addLog("Scheduler → Closing (scheduled)");
            }
        }
    }
}

// ---------------------------------------------------------
// Initialization
// ---------------------------------------------------------
void SchedulerTask_begin() {

    if (ctx) return;

    ctx = new SchedulerContext();

    if (!s_schedulerTaskHandle) {
        xTaskCreatePinnedToCore(
            SchedulerTask,
            "SchedulerTask",
            4096,
            nullptr,
            2,
            &s_schedulerTaskHandle,
            1
        );
        addLog("Scheduler → Task started");
    }
}
