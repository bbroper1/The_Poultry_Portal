#include "modules/hardware/HardwarePins.h"
#include "modules/motor/MotorPins.h"   // for PIN_LIMIT_* and PIN_SWITCH_*
#include "modules/system/Logging.h"

// Heartbeat LED pin
static const int PIN_HEARTBEAT = 2;

// ---------------------------------------------------------
// Safe pinMode wrapper (ignores -1 or invalid pins)
// ---------------------------------------------------------
static void safePinMode(int pin, uint8_t mode) {
    if (pin >= 0 && pin <= 39) {
        pinMode(pin, mode);
    }
}

static void safeDigitalWrite(int pin, uint8_t val) {
    if (pin >= 0 && pin <= 39) {
        digitalWrite(pin, val);
    }
}

// ---------------------------------------------------------
// Initialization
// ---------------------------------------------------------
void HardwarePins_begin() {
    // Limit switches
    safePinMode(PIN_LIMIT_OPEN, INPUT_PULLUP);
    safePinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);

    // Manual override switches
    safePinMode(PIN_SWITCH_OPEN, INPUT_PULLUP);
    safePinMode(PIN_SWITCH_CLOSE, INPUT_PULLUP);

    // Heartbeat LED
    safePinMode(PIN_HEARTBEAT, OUTPUT);
    safeDigitalWrite(PIN_HEARTBEAT, LOW);

    addLog("[Hardware] Pins initialized");
    addLog("[Hardware] LimitOpen=" + String(PIN_LIMIT_OPEN) +
           " LimitClose=" + String(PIN_LIMIT_CLOSE) +
           " SwitchOpen=" + String(PIN_SWITCH_OPEN) +
           " SwitchClose=" + String(PIN_SWITCH_CLOSE) +
           " Heartbeat=" + String(PIN_HEARTBEAT));
}

// ---------------------------------------------------------
// Heartbeat helpers
// ---------------------------------------------------------
void HardwarePins_heartbeatOn() {
    safeDigitalWrite(PIN_HEARTBEAT, HIGH);
}

void HardwarePins_heartbeatOff() {
    safeDigitalWrite(PIN_HEARTBEAT, LOW);
}

void HardwarePins_heartbeatToggle() {
    int v = digitalRead(PIN_HEARTBEAT);
    safeDigitalWrite(PIN_HEARTBEAT, !v);
}

void HardwarePins_blink(int times, int ms) {
    for (int i = 0; i < times; i++) {
        HardwarePins_heartbeatOn();
        delay(ms);
        HardwarePins_heartbeatOff();
        delay(ms);
    }
}

int HardwarePins_getHeartbeatPin() {
    return PIN_HEARTBEAT;
}
