#include "StringUtils.h"

bool StringUtils_isNumber(const String &s) {
    if (s.length() == 0) return false;

    for (int i = 0; i < s.length(); i++) {
        char c = s[i];

        // Allow leading + or -
        if (i == 0 && (c == '-' || c == '+')) continue;

        if (!isDigit(c)) return false;
    }

    return true;
}
