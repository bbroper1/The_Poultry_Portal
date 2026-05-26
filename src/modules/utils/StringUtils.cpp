#include "modules/utils/StringUtils.h"

namespace StringUtils {

// ---------------------------------------------------------
// Basic digit-only check
// ---------------------------------------------------------
bool isNumber(const String& s) {
    if (s.length() == 0) return false;

    for (size_t i = 0; i < s.length(); ++i) {
        if (!isDigit(s[i])) return false;
    }
    return true;
}

// ---------------------------------------------------------
// Signed integer check: -12, +5, 42
// ---------------------------------------------------------
bool isSignedNumber(const String& s) {
    if (s.length() == 0) return false;

    size_t start = 0;
    if (s[0] == '-' || s[0] == '+') {
        if (s.length() == 1) return false; // just "+" or "-"
        start = 1;
    }

    for (size_t i = start; i < s.length(); ++i) {
        if (!isDigit(s[i])) return false;
    }
    return true;
}

// ---------------------------------------------------------
// Float check: 12.5, -3.14, +0.99
// ---------------------------------------------------------
bool isFloat(const String& s) {
    if (s.length() == 0) return false;

    bool seenDot = false;
    size_t start = 0;

    if (s[0] == '-' || s[0] == '+') {
        if (s.length() == 1) return false;
        start = 1;
    }

    for (size_t i = start; i < s.length(); ++i) {
        if (s[i] == '.') {
            if (seenDot) return false;
            seenDot = true;
            continue;
        }
        if (!isDigit(s[i])) return false;
    }

    return true;
}

// ---------------------------------------------------------
// Trim whitespace
// ---------------------------------------------------------
String trim(const String& s) {
    int start = 0;
    int end   = s.length() - 1;

    while (start <= end && isspace(s[start])) start++;
    while (end >= start && isspace(s[end]))   end--;

    if (start > end) return "";
    return s.substring(start, end + 1);
}

// ---------------------------------------------------------
// Safe conversions
// ---------------------------------------------------------
int toIntSafe(const String& s, int defaultValue) {
    if (!isSignedNumber(trim(s))) return defaultValue;
    return trim(s).toInt();
}

float toFloatSafe(const String& s, float defaultValue) {
    if (!isFloat(trim(s))) return defaultValue;
    return trim(s).toFloat();
}

} // namespace StringUtils
