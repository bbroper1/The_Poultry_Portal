#include "modules/energy/EnergyModule.h"
#include "modules/time/TimeManager.h"
#include <Preferences.h>
#include <time.h>

// ---------------------------------------------------------
// MODULE STATE
// ---------------------------------------------------------
static Preferences prefs;

static float today_mAh    = 0.0f;
static float avgDaily_mAh = 0.0f;
static float monthly_mAh  = 0.0f;

static float history[30] = {0};
static time_t lastReset  = 0;

// ---------------------------------------------------------
// INITIALIZATION
// ---------------------------------------------------------
void Energy_begin() {
    prefs.begin("energy", false);

    today_mAh    = prefs.getFloat("today", 0.0f);
    avgDaily_mAh = prefs.getFloat("avg",   0.0f);
    monthly_mAh  = prefs.getFloat("month", 0.0f);

    prefs.getBytes("hist", history, sizeof(history));
    lastReset = prefs.getULong("last", 0);
}

// ---------------------------------------------------------
// ADD ENERGY USAGE
// ---------------------------------------------------------
void Energy_addmAh(float mAh) {
    today_mAh += mAh;
}

// ---------------------------------------------------------
// DAILY ROLLOVER
// ---------------------------------------------------------
void Energy_update() {
    if (!TimeManager::isValid())
        return;

    time_t now = time(nullptr);

    struct tm t = TimeManager::getLocalTime();
    struct tm r = *localtime(&lastReset);   // lastReset is a timestamp

    if (t.tm_mday != r.tm_mday) {

        // Shift history
        for (int i = 29; i > 0; i--) {
            history[i] = history[i - 1];
        }
        history[0] = today_mAh;

        // Recompute 30-day average
        float sum = 0;
        for (int i = 0; i < 30; i++) sum += history[i];
        avgDaily_mAh = sum / 30.0f;

        // Monthly accumulation
        monthly_mAh += today_mAh;

        // Reset daily
        today_mAh = 0;
        lastReset = now;

        Energy_forceSave();
    }
}

// ---------------------------------------------------------
// ACCESSORS
// ---------------------------------------------------------
float Energy_getTodaymAh()        { return today_mAh; }
float Energy_getAvgDailymAh()     { return avgDaily_mAh; }
float Energy_getMonthlymAh()      { return monthly_mAh; }
time_t Energy_getLastResetTimestamp() { return lastReset; }

String Energy_getLastResetStr() {
    if (lastReset == 0) return "N/A";

    struct tm tmInfo = *localtime(&lastReset);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tmInfo);
    return String(buf);
}

float* Energy_getHistoryArray() {
    return history;
}

// ---------------------------------------------------------
// MANUAL CONTROL
// ---------------------------------------------------------
void Energy_resetDaily() {
    today_mAh = 0;
    lastReset = time(nullptr);
    Energy_forceSave();
}

void Energy_resetMonthly() {
    monthly_mAh = 0;
    Energy_forceSave();
}

void Energy_forceSave() {
    prefs.putFloat("today", today_mAh);
    prefs.putFloat("avg",   avgDaily_mAh);
    prefs.putFloat("month", monthly_mAh);
    prefs.putBytes("hist",  history, sizeof(history));
    prefs.putULong("last",  lastReset);
}

void Energy_forceLoad() {
    Energy_begin();
}