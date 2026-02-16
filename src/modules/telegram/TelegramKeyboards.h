#pragma once
#include <Arduino.h>

namespace Keyboards {

    // ---------------------------------------------------------
    // MAIN MENU
    // ---------------------------------------------------------
    inline String mainMenu() {
        return
            "["
            "  [\"/status\", \"/settings\"],"
            "  [\"/debug\", \"/timezone\"]"
            "]";
    }

    // ---------------------------------------------------------
    // SETTINGS MENU
    // ---------------------------------------------------------
    inline String settingsMenu() {
        return
            "["
            "  [\"/auto\", \"/open\"],"
            "  [\"/close\", \"/back\"]"
            "]";
    }

    // ---------------------------------------------------------
    // DEBUG MENU
    // ---------------------------------------------------------
    inline String debugMenu() {
        return
            "["
            "  [\"/heap\", \"/motortest\"],"
            "  [\"/wifi\", \"/back\"]"
            "]";
    }

    // ---------------------------------------------------------
    // TIMEZONE MENU
    // ---------------------------------------------------------
    inline String timezoneMenu() {
        return
            "["
            "  [\"/cst\", \"/est\"],"
            "  [\"/pst\", \"/utc\"],"
            "  [\"/back\"]"
            "]";
    }

} // namespace Keyboards