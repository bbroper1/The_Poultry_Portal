#pragma once
#include <Arduino.h>

// Start the FreeRTOS scheduler task
void SchedulerTask_begin();

// Sunrise/sunset helpers
String Scheduler_getNextOpen();
String Scheduler_getNextClose();
void Scheduler_calcLocalSunTimes(int &sunriseLocal, int &sunsetLocal);