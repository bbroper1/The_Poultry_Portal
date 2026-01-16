#pragma once
#include <Arduino.h>

// Initialize scheduler (optional for future expansion)
void Scheduler_init();

// Returns formatted next open time string
String Scheduler_getNextOpen();

// Returns formatted next close time string
String Scheduler_getNextClose();

// Internal helper if needed elsewhere
void Scheduler_calcLocalSunTimes(int &sunriseLocal, int &sunsetLocal);
