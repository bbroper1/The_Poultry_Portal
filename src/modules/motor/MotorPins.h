#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Relay H-bridge pins (2‑relay module)
// ---------------------------------------------------------
// IMPORTANT: These two pins must always be driven together
// (00 = stop, 11 = run). MotorController enforces this.
extern int PIN_MOTOR_A;   // Relay IN1 (GPIO 26)
extern int PIN_MOTOR_B;   // Relay IN2 (GPIO 18)

// ---------------------------------------------------------
// Magnetic limit switches (Hall sensors)
// ---------------------------------------------------------
// These are wired to output HIGH when magnet is present.
extern int PIN_LIMIT_OPEN;    // Top limit (GPIO 33)
extern int PIN_LIMIT_CLOSE;   // Bottom limit (GPIO 32)

// ---------------------------------------------------------
// Manual override switches (momentary buttons)
// ---------------------------------------------------------
extern int PIN_SWITCH_OPEN;   // Manual open button (GPIO 14)
extern int PIN_SWITCH_CLOSE;  // Manual close button (GPIO 27)
