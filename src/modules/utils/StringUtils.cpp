#include "modules/utils/StringUtils.h"

namespace StringUtils {

bool isNumber(const String& s) {
    if (s.length() == 0) return false;

    for (size_t i = 0; i < s.length(); ++i) {
        if (!isDigit(s[i])) {
            return false;
        }
    }
    return true;
}

}