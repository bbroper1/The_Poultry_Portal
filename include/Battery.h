#pragma once
#include <Arduino.h>

void Battery_begin();
void Battery_update();

float Battery_getVoltage();
float Battery_getCurrent();

bool Battery_isLow();
bool Battery_isCritical();
bool Battery_alertSent();

float Battery_getWarningVoltage();
float Battery_getCriticalVoltage();

void Battery_begin();
