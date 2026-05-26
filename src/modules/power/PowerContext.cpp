#include "PowerContext.h"

// ---------------------------------------------------------
// Global telemetry values (legacy)
// ---------------------------------------------------------

float inputVoltage  = 0.0f;
float outputVoltage = 0.0f;

float avgOpenTime  = 0.0f;
float avgCloseTime = 0.0f;

// Battery cutoff threshold (constant)
const float CRITICAL_VOLTAGE = 10.5f;

// INA219 status + last measured motor current
bool  inaOK        = false;
float motorCurrent = 0.0f;

// ---------------------------------------------------------
// Optional initialization
// ---------------------------------------------------------
void PowerContext_begin() {
    inputVoltage  = 0.0f;
    outputVoltage = 0.0f;

    avgOpenTime  = 0.0f;
    avgCloseTime = 0.0f;

    inaOK        = false;
    motorCurrent = 0.0f;
}
