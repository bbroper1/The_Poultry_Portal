#include "modules/energy/EnergyModule.h"
#include "modules/time/TimeManager.h"
#include "modules/system/Logging.h"
#include <Preferences.h>
#include <time.h>

// ---------------------------------------------------------
// INTERNAL STRUCTURE FOR ONE ENERGY CHANNEL
// ---------------------------------------------------------
struct EnergyChannel {
    Preferences prefs;

    float today_mAh    = 0.0f;
    float avgDaily_mAh = 0.0f;
    float monthly_mAh  = 0.0f;

    float history[30] = {0};
    time_t lastReset  = 0;

    int peakCurrent   = 0;
    uint32_t currentSum = 0;
    uint32_t currentSamples = 0;

    String ns;   // namespace
};

// Two channels: system + motor
static EnergyChannel sysCh;
static EnergyChannel motCh;

// ---------------------------------------------------------
// LOAD / SAVE HELPERS
// ---------------------------------------------------------
static void loadChannel(EnergyChannel& ch) {
    if (ch.ns.length() == 0) return; // safety

    ch.prefs.begin(ch.ns.c_str(), false);

    ch.today_mAh    = ch.prefs.getFloat("today", 0.0f);
    ch.avgDaily_mAh = ch.prefs.getFloat("avg",   0.0f);
    ch.monthly_mAh  = ch.prefs.getFloat("month", 0.0f);

    ch.prefs.getBytes("hist", ch.history, sizeof(ch.history));
    ch.lastReset = ch.prefs.getULong("last", 0);

    ch.peakCurrent     = ch.prefs.getInt("peak", 0);
    ch.currentSum      = ch.prefs.getULong("csum", 0);
    ch.currentSamples  = ch.prefs.getULong("csamp", 0);

    // Protect against corrupted flash
    for (int i = 0; i < 30; i++) {
        if (!isfinite(ch.history[i]) || ch.history[i] < 0)
            ch.history[i] = 0;
    }
}

static void saveChannel(EnergyChannel& ch) {
    if (ch.ns.length() == 0) return;

    ch.prefs.putFloat("today", ch.today_mAh);
    ch.prefs.putFloat("avg",   ch.avgDaily_mAh);
    ch.prefs.putFloat("month", ch.monthly_mAh);
    ch.prefs.putBytes("hist",  ch.history, sizeof(ch.history));
    ch.prefs.putULong("last",  ch.lastReset);

    ch.prefs.putInt("peak",    ch.peakCurrent);
    ch.prefs.putULong("csum",  ch.currentSum);
    ch.prefs.putULong("csamp", ch.currentSamples);
}

// ---------------------------------------------------------
// DAILY ROLLOVER
// ---------------------------------------------------------
static void updateChannel(EnergyChannel& ch) {
    if (!TimeManager::isValid())
        return;

    time_t now = time(nullptr);
    if (ch.lastReset == 0) {
        ch.lastReset = now;
        saveChannel(ch);
        return;
    }

    struct tm nowTm = TimeManager::getLocalTime();
    struct tm lastTm = *localtime(&ch.lastReset);

    // If day changed
    if (nowTm.tm_year != lastTm.tm_year ||
        nowTm.tm_mon  != lastTm.tm_mon  ||
        nowTm.tm_mday != lastTm.tm_mday) {

        addLog(String("Energy → Daily reset for ") + ch.ns);

        // Shift history
        for (int i = 29; i > 0; i--)
            ch.history[i] = ch.history[i - 1];

        ch.history[0] = ch.today_mAh;

        // Recompute average (only non-zero days)
        float sum = 0;
        int count = 0;
        for (int i = 0; i < 30; i++) {
            if (ch.history[i] > 0.01f) {
                sum += ch.history[i];
                count++;
            }
        }
        ch.avgDaily_mAh = (count > 0) ? (sum / count) : 0;

        // Monthly accumulation
        ch.monthly_mAh += ch.today_mAh;

        // Reset daily
        ch.today_mAh = 0;
        ch.peakCurrent = 0; // NEW
        ch.currentSum = 0;
        ch.currentSamples = 0;

        ch.lastReset = now;

        saveChannel(ch);
    }
}

// ---------------------------------------------------------
// PUBLIC API — SYSTEM ENERGY
// ---------------------------------------------------------
void EnergySys_begin() { loadChannel(sysCh); }
void EnergySys_update() { updateChannel(sysCh); }
void EnergySys_addmAh(float mAh) { sysCh.today_mAh += mAh; }

float  EnergySys_getTodaymAh()        { return sysCh.today_mAh; }
float  EnergySys_getAvgDailymAh()     { return sysCh.avgDaily_mAh; }
float  EnergySys_getMonthlymAh()      { return sysCh.monthly_mAh; }
time_t EnergySys_getLastResetTimestamp() { return sysCh.lastReset; }
float* EnergySys_getHistoryArray()    { return sysCh.history; }

String EnergySys_getLastResetStr() {
    if (sysCh.lastReset == 0) return "N/A";
    struct tm tmInfo = *localtime(&sysCh.lastReset);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tmInfo);
    return String(buf);
}

int EnergySys_getPeakCurrentmA() { return sysCh.peakCurrent; }
int EnergySys_getAvgCurrentmA() {
    return (sysCh.currentSamples == 0) ? 0 : (sysCh.currentSum / sysCh.currentSamples);
}

void EnergySys_resetDaily() { sysCh.today_mAh = 0; sysCh.lastReset = time(nullptr); saveChannel(sysCh); }
void EnergySys_resetMonthly() { sysCh.monthly_mAh = 0; saveChannel(sysCh); }
void EnergySys_forceSave() { saveChannel(sysCh); }
void EnergySys_forceLoad() { loadChannel(sysCh); }

// ---------------------------------------------------------
// PUBLIC API — MOTOR ENERGY
// ---------------------------------------------------------
void EnergyMotor_begin() { loadChannel(motCh); }
void EnergyMotor_update() { updateChannel(motCh); }
void EnergyMotor_addmAh(float mAh) { motCh.today_mAh += mAh; }

float  EnergyMotor_getTodaymAh()        { return motCh.today_mAh; }
float  EnergyMotor_getAvgDailymAh()     { return motCh.avgDaily_mAh; }
float  EnergyMotor_getMonthlymAh()      { return motCh.monthly_mAh; }
time_t EnergyMotor_getLastResetTimestamp() { return motCh.lastReset; }
float* EnergyMotor_getHistoryArray()    { return motCh.history; }

String EnergyMotor_getLastResetStr() {
    if (motCh.lastReset == 0) return "N/A";
    struct tm tmInfo = *localtime(&motCh.lastReset);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tmInfo);
    return String(buf);
}

int EnergyMotor_getPeakCurrentmA() { return motCh.peakCurrent; }
int EnergyMotor_getAvgCurrentmA() {
    return (motCh.currentSamples == 0) ? 0 : (motCh.currentSum / motCh.currentSamples);
}

void EnergyMotor_resetDaily() { motCh.today_mAh = 0; motCh.lastReset = time(nullptr); saveChannel(motCh); }
void EnergyMotor_resetMonthly() { motCh.monthly_mAh = 0; saveChannel(motCh); }
void EnergyMotor_forceSave() { saveChannel(motCh); }
void EnergyMotor_forceLoad() { loadChannel(motCh); }

// ---------------------------------------------------------
// COMBINED API
// ---------------------------------------------------------
void Energy_begin() {
    sysCh.ns = "energy_sys";
    motCh.ns = "energy_motor";

    EnergySys_begin();
    EnergyMotor_begin();
}

void Energy_update() {
    EnergySys_update();
    EnergyMotor_update();
}
