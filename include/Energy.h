#pragma once
#include "Energy.h"
#include <Arduino.h>
#include <time.h>

// Initialize energy tracking (call once from setup)
void Energy_begin();

// Update energy tracking and daily rollover (call from loop)
void Energy_update();

// Read-only accessors
float Energy_getTodaymAh();
float Energy_getAvgDailymAh();
float Energy_getMonthlymAh();
String Energy_getLastResetStr();
time_t Energy_getLastResetTimestamp();
float* Energy_getHistoryArray();

// Setters used by Battery_begin()
void Energy_setTodaymAh(float v);
void Energy_setLastReset(time_t t);

// Manual control APIs
void Energy_resetDaily();
void Energy_resetMonthly();
void Energy_forceSave();
void Energy_forceLoad();
