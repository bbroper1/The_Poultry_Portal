#include "modules/AutoMode/AutoModeTask.h"
#include "modules/config/Config.h"
#include "modules/motor/MotorTask.h"
#include "modules/system/Logging.h"
#include "modules/scheduler/SunContext.h"
#include "modules/time/TimeManager.h"

#include <sunset.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t s_autoModeTaskHandle = nullptr;

// ---------------------------------------------------------
// External globals
// ---------------------------------------------------------
extern SunSet sun;
extern bool remoteOverride;
extern time_t remoteOverrideUntil;

// ---------------------------------------------------------
// Helper: is override active?
// ---------------------------------------------------------
bool AutoMode_isOverrideActive() {
    time_t now = time(nullptr);
    return (remoteOverride && now < remoteOverrideUntil);
}

// ---------------------------------------------------------
// Sunrise/Sunset Calculation
// ---------------------------------------------------------
static bool computeShouldBeOpen(bool& out) {
    if (!TimeManager::isValid()) {
        addLog("AutoMode → Time invalid, skipping");
        return false;
    }

    struct tm nowTm = TimeManager::getLocalTime();

    sun.setCurrentDate(
        nowTm.tm_year + 1900,
        nowTm.tm_mon + 1,
        nowTm.tm_mday
    );

    sun.setPosition(
        Config_getLat(),
        Config_getLong(),
        TimeManager::utcOffsetHours()
    );

    int sunrise    = (int)sun.calcSunrise() + Config_getOpenOffset();
    int sunset     = (int)sun.calcSunset()  + Config_getCloseOffset();
    int minutesNow = nowTm.tm_hour * 60 + nowTm.tm_min;

    out = (minutesNow >= sunrise && minutesNow < sunset);
    return true;
}

// ---------------------------------------------------------
// Boot Correction (one-shot)
// ---------------------------------------------------------
void AutoMode_bootCorrection() {
    if (AutoMode_isOverrideActive()) {
        addLog("Auto Boot → Skipped (manual override active)");
        return;
    }

    bool shouldBeOpen = false;
    if (!computeShouldBeOpen(shouldBeOpen)) {
        addLog("Auto Boot → Time invalid, skipping");
        return;
    }

    MotorDoorState state = Motor_getState();

    if (shouldBeOpen) {
        if (state != M_OPEN && state != M_OPENING) {
            Motor_requestOpen();
            addLog("Auto Boot → Opening (should be open)");
        } else {
            addLog("Auto Boot → Already open");
        }
    } else {
        if (state != M_CLOSED && state != M_CLOSING) {
            Motor_requestClose();
            addLog("Auto Boot → Closing (should be closed)");
        } else {
            addLog("Auto Boot → Already closed");
        }
    }
}

// ---------------------------------------------------------
// AutoMode FreeRTOS Task
// ---------------------------------------------------------
static void AutoModeTask(void* pv) {
    setenv("TZ", Config_getTimezone().c_str(), 1);
    tzset();

    const TickType_t interval = pdMS_TO_TICKS(60000); // 1 minute
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, interval);

        time_t now = time(nullptr);

        // -------------------------------------------------
        // Handle override expiration
        // -------------------------------------------------
        if (remoteOverride && now >= remoteOverrideUntil) {
            remoteOverride = false;
            addLog("AutoMode → Override expired, resuming AutoMode");
        }

        // -------------------------------------------------
        // Skip AutoMode while override is active
        // -------------------------------------------------
        if (AutoMode_isOverrideActive()) {
            int secLeft = remoteOverrideUntil - now;
            if (secLeft < 0) secLeft = 0;

            int minLeft = secLeft / 60;
            addLog("AutoMode → Override active (" + String(minLeft) + "m left)");
            continue;
        }

        // -------------------------------------------------
        // Normal AutoMode logic
        // -------------------------------------------------
        bool shouldBeOpen = false;
        if (!computeShouldBeOpen(shouldBeOpen))
            continue;

        MotorDoorState state = Motor_getState();

        if (shouldBeOpen) {
            if (state != M_OPEN && state != M_OPENING) {
                Motor_requestOpen();
                addLog("AutoMode → Opening (scheduled)");
            }
        } else {
            if (state != M_CLOSED && state != M_CLOSING) {
                Motor_requestClose();
                addLog("AutoMode → Closing (scheduled)");
            }
        }
    }
}

// ---------------------------------------------------------
// Initialization
// ---------------------------------------------------------
void AutoMode_begin() {
    static bool started = false;
    if (started) return;
    started = true;

    xTaskCreatePinnedToCore(
        AutoModeTask,
        "AutoModeTask",
        4096,
        nullptr,
        2,      // lower priority than MotorTask
        nullptr,
        1       // same core as MotorTask
    );

    addLog("AutoMode → Task started");
}
