#include "PowerContext.h"

float inputVoltage = 0.0f;
float outputVoltage = 0.0f;

float avgOpenTime = 0.0f;
float avgCloseTime = 0.0f;

float CRITICAL_VOLTAGE = 10.5f;

bool inaOK = false;
float motorCurrent = 0.0f;