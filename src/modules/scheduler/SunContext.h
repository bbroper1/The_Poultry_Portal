#pragma once
#include <sunset.h>

// Global SunSet instance used by SchedulerTask and AutoModeTask
extern SunSet sun;

// Initialize SunSet with config values (lat, lon, timezone)
void SunContext_begin();

// Optional: refresh after timezone/location changes
void SunContext_refresh();
