#include "HardwarePins.h"

// -------------------------------
//  DEFAULT PIN ASSIGNMENTS
// -------------------------------

// Motor driver pins
int PIN_MOTOR_A = 25;
int PIN_MOTOR_B = 26;

// Limit switches
int PIN_LIMIT_OPEN  = 32;
int PIN_LIMIT_CLOSE = 33;

// Manual override switches
int PIN_SWITCH_OPEN  = 14;
int PIN_SWITCH_CLOSE = 27;

// Heartbeat LED
int PIN_HEARTBEAT = 2;

// PWM channels
int PWM_CH_A = 0;
int PWM_CH_B = 1;

void HardwarePins_init() {
    // Motor outputs
    pinMode(PIN_MOTOR_A, OUTPUT);
    pinMode(PIN_MOTOR_B, OUTPUT);

    // Limit switches
    pinMode(PIN_LIMIT_OPEN, INPUT_PULLUP);
    pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);

    // Manual override switches
    pinMode(PIN_SWITCH_OPEN, INPUT_PULLUP);
    pinMode(PIN_SWITCH_CLOSE, INPUT_PULLUP);

    // Heartbeat LED
    pinMode(PIN_HEARTBEAT, OUTPUT);
    digitalWrite(PIN_HEARTBEAT, LOW);
}
