#include "MotorPins.h"
#include "modules/system/Logging.h"

// ---------------------------------------------------------
// Relay H-bridge pins
// ---------------------------------------------------------
int PIN_MOTOR_A = 26;   // Relay IN1
int PIN_MOTOR_B = 18;   // Relay IN2

// ---------------------------------------------------------
// Magnetic limit switches
// ---------------------------------------------------------
int PIN_LIMIT_OPEN  = 33;   // Top limit sensor
int PIN_LIMIT_CLOSE = 32;   // Bottom limit sensor

// ---------------------------------------------------------
// Manual override switches
// ---------------------------------------------------------
int PIN_SWITCH_OPEN  = 14;
int PIN_SWITCH_CLOSE = 27;

// ---------------------------------------------------------
// Optional: log pin assignments at boot
// ---------------------------------------------------------
__attribute__((constructor))
static void logMotorPins() {
    addLog("[MotorPins] A=" + String(PIN_MOTOR_A) +
           " B=" + String(PIN_MOTOR_B) +
           " LimitOpen=" + String(PIN_LIMIT_OPEN) +
           " LimitClose=" + String(PIN_LIMIT_CLOSE) +
           " SwitchOpen=" + String(PIN_SWITCH_OPEN) +
           " SwitchClose=" + String(PIN_SWITCH_CLOSE));
}
