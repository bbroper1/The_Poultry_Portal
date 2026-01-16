#pragma once
#include <Arduino.h>

// Returns true if the string represents a valid integer (+/- allowed)
bool StringUtils_isNumber(const String &s);
