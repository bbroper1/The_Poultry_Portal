#pragma once
#include <Arduino.h>

// Start the FreeRTOS scheduler task
void SchedulerTask_begin();

// Sunrise/sunset helpers
void   Scheduler_calcLocalSunTimes(int &sunriseLocal, int &sunsetLocal);
String Scheduler_getNextOpen();
String Scheduler_getNextClose();

// Optional helper
bool Scheduler_shouldBeOpenNow();

extern TaskHandle_t s_schedulerTaskHandle;
