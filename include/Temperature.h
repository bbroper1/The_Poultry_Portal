#pragma once
#include <Arduino.h>

// Initialize temperature system (if needed)
void Temperature_begin();

// Call regularly from loop()
void Temperature_update();

// Accessors
float Temperature_getCelsius();
String Temperature_getLabel();   // "OK", "⚠️ HIGH", "🔥 CRITICAL"

bool Temperature_isCritical();
bool Temperature_isWarning();

float Temperature_getCriticalC();
float Temperature_getWarningC();
