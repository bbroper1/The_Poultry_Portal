#pragma once
#include <Arduino.h>

// Returns the UTC offset in hours based on the configured timezone string.
// Example: "UTC+2" → +2.0, "GMT-5" → -5.0, "CST6CDT" → -6.0 (approx)
int TimeUtils_getUTCOffsetHours();
bool TimeUtils_timeIsValid();
void TimeUtils_sync();
String TimeUtils_getUptime();
