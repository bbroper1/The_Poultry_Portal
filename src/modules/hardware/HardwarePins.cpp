#include "modules/hardware/HardwarePins.h"
#include "modules/motor/MotorPins.h"   // for PIN_LIMIT_* and PIN_SWITCH_*
#include "modules/system/Logging.h"

// Heartbeat LED pin
static const int PIN_HEARTBEAT = 2;

void HardwarePins_begin() {
    // Limit switches
    pinMode(PIN_LIMIT_OPEN, INPUT_PULLUP);
    pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);

    // Manual override switches
    pinMode(PIN_SWITCH_OPEN, INPUT_PULLUP);
    pinMode(PIN_SWITCH_CLOSE, INPUT_PULLUP);

    // Heartbeat LED
    pinMode(PIN_HEARTBEAT, OUTPUT);
    digitalWrite(PIN_HEARTBEAT, LOW);

    addLog("[Hardware] Pins initialized");
}