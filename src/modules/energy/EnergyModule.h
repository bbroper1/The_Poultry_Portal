#pragma once
#include <Arduino.h>
#include <time.h>

// Initialize energy tracking (load from Preferences)
void Energy_begin();

// Update daily rollover (called once per minute or per SensorTask tick)
void Energy_update();

// Add mAh consumption (called by SensorTask)
void Energy_addmAh(float mAh);

// Read-only accessors
float  Energy_getTodaymAh();
float  Energy_getAvgDailymAh();
float  Energy_getMonthlymAh();
String Energy_getLastResetStr();
time_t Energy_getLastResetTimestamp();
float* Energy_getHistoryArray();

// Manual control
void Energy_resetDaily();
void Energy_resetMonthly();
void Energy_forceSave();
void Energy_forceLoad();