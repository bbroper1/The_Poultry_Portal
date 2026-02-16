#pragma once

// ---------------------------------------------------------
// PWM settings for DRV8871
// ---------------------------------------------------------
#define PWM_FREQ        20000      // 20 kHz
#define PWM_RESOLUTION  10         // 10-bit resolution (0–1023)

// ---------------------------------------------------------
// DRV8871 motor driver pins
// ---------------------------------------------------------
extern int PIN_MOTOR_A;     // IN1 (GPIO 18)
extern int PIN_MOTOR_B;     // IN2 (GPIO 26)

// ---------------------------------------------------------
// Limit switches
// ---------------------------------------------------------
extern int PIN_LIMIT_OPEN;   // GPIO 33
extern int PIN_LIMIT_CLOSE;  // GPIO 32

// ---------------------------------------------------------
// Manual override switches
// ---------------------------------------------------------
extern int PIN_SWITCH_OPEN;   // GPIO 14
extern int PIN_SWITCH_CLOSE;  // GPIO 27