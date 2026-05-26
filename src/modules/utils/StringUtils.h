#pragma once
#include <Arduino.h>

namespace StringUtils {

    // Returns true if the string contains only digits (0–9)
    bool isNumber(const String& s);

    // Returns true for signed integers: -12, +5, 42
    bool isSignedNumber(const String& s);

    // Returns true for floats: 12.5, -3.14, +0.99
    bool isFloat(const String& s);

    // Trim whitespace from both ends
    String trim(const String& s);

    // Safe conversions (return default on failure)
    int    toIntSafe(const String& s, int defaultValue = 0);
    float  toFloatSafe(const String& s, float defaultValue = 0.0f);
}
