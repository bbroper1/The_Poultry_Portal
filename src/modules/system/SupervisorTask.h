#pragma once
#include <Arduino.h>

// Start the Supervisor FreeRTOS task
void SupervisorTask_begin();

// Optional helpers
void SupervisorTask_checkHealth();
void SupervisorTask_sendAlerts();