#pragma once
#include <Arduino.h>

namespace StringUtils {

    // Returns true if the string contains only digits (0–9)
    bool isNumber(const String& s);

}