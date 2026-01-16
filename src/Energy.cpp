#include "Energy.h"
#include <Preferences.h>
#include <time.h>
#include "Motor.h"

// -------------------------------
// Internal module state
// -------------------------------
static float   s_todayUsedmAh = 0.0f;
static float   s_historymAh[30] = {0};
static unsigned long s_lastEnergyCalc = 0;
static time_t  s_lastResetTimestamp = 0;
static String  s_lastDailyResetStr = "Never";

static Preferences s_energyPrefs;  // namespace: "energy"

// -------------------------------
// Helpers
// -------------------------------
static bool timeIsValidInternal() {
    time_t now = time(nullptr);
    return now > 1700000000; // after 2023
}

// -------------------------------
// Public getters
// -------------------------------
float Energy_getTodaymAh() {
    return s_todayUsedmAh;
}

float Energy_getAvgDailymAh() {
    float sum = 0;
    int count = 0;

    for (int i = 0; i < 30; i++) {
        if (s_historymAh[i] > 0.1f) {
            sum += s_historymAh[i];
            count++;
        }
    }

    if (count == 0) return 0;
    return sum / count;
}

float Energy_getMonthlymAh() {
    float sum = 0;
    for (int i = 0; i < 30; i++) {
        sum += s_historymAh[i];
    }
    return sum;
}

String Energy_getLastResetStr() {
    return s_lastDailyResetStr;
}

time_t Energy_getLastResetTimestamp() {
    return s_lastResetTimestamp;
}

float* Energy_getHistoryArray() {
    return s_historymAh;
}

// -------------------------------
// Setters used by Battery_begin()
// -------------------------------
void Energy_setTodaymAh(float v) {
    s_todayUsedmAh = v;
}

void Energy_setLastReset(time_t t) {
    s_lastResetTimestamp = t;

    struct tm* ptm = localtime(&t);
    if (ptm) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", ptm->tm_hour, ptm->tm_min);
        s_lastDailyResetStr = String(buf);
    } else {
        s_lastDailyResetStr = "Never";
    }
}

// -------------------------------
// Persistence
// -------------------------------
void Energy_loadFromPrefs() {
    if (!s_energyPrefs.begin("energy", true)) return;

    s_todayUsedmAh = s_energyPrefs.getFloat("today", 0.0f);
    s_lastResetTimestamp = s_energyPrefs.getULong("lastreset", 0);

    if (s_energyPrefs.isKey("history")) {
        size_t sz = s_energyPrefs.getBytesLength("history");
        if (sz == sizeof(s_historymAh)) {
            s_energyPrefs.getBytes("history", s_historymAh, sizeof(s_historymAh));
        }
    }

    s_energyPrefs.end();

    // sanitize
    if (isnan(s_todayUsedmAh) || isinf(s_todayUsedmAh)) s_todayUsedmAh = 0.0f;

    for (int i = 0; i < 30; i++) {
        if (isnan(s_historymAh[i]) || isinf(s_historymAh[i])) {
            s_historymAh[i] = 0.0f;
        }
    }

    // update string
    Energy_setLastReset(s_lastResetTimestamp);
}

void Energy_saveToPrefs() {
    if (!s_energyPrefs.begin("energy", false)) return;

    s_energyPrefs.putFloat("today", s_todayUsedmAh);
    s_energyPrefs.putULong("lastreset", s_lastResetTimestamp);
    s_energyPrefs.putBytes("history", s_historymAh, sizeof(s_historymAh));

    s_energyPrefs.end();
}

// -------------------------------
// Daily rollover
// -------------------------------
static void Energy_checkDailyResetInternal() {
    if (!timeIsValidInternal()) return;

    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);
    if (!ptm) return;

    static int s_lastDay = -1;
    int today = ptm->tm_yday;

    if (s_lastDay == -1) {
        s_lastDay = today;
        return;
    }

    if (today != s_lastDay) {
        s_lastDay = today;

        // shift history
        for (int i = 29; i > 0; i--) {
            s_historymAh[i] = s_historymAh[i - 1];
        }
        s_historymAh[0] = s_todayUsedmAh;

        // reset daily
        s_todayUsedmAh = 0.0f;

        // update timestamp + string
        Energy_setLastReset(now);

        Energy_saveToPrefs();
    }
}

// -------------------------------
// Lifecycle
// -------------------------------
void Energy_begin() {
    s_lastEnergyCalc = millis();
    Energy_loadFromPrefs();
}

void Energy_update() {
    unsigned long now = millis();

    // Protect against rollover or negative dt
    if (now < s_lastEnergyCalc) {
        s_lastEnergyCalc = now;
        return;
    }

    unsigned long dt = now - s_lastEnergyCalc;

    if (dt >= 1000) {
        float mA = Motor_getCurrent();

        // Convert dt to hours
        float dtHours = dt / 3600000.0f;

        // Accumulate mAh
        s_todayUsedmAh += mA * dtHours;

        s_lastEnergyCalc = now;
    }

    // daily rollover
    Energy_checkDailyResetInternal();
}

// -------------------------------
// Manual control APIs
// -------------------------------
void Energy_resetDaily() {
    s_todayUsedmAh = 0.0f;
    time_t now = time(nullptr);
    Energy_setLastReset(now);
    Energy_saveToPrefs();
}

void Energy_resetMonthly() {
    for (int i = 0; i < 30; i++) {
        s_historymAh[i] = 0.0f;
    }
    Energy_saveToPrefs();
}

void Energy_forceSave() {
    Energy_saveToPrefs();
}

void Energy_forceLoad() {
    Energy_loadFromPrefs();
}
