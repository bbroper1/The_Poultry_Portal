#include "MotorPins.h"

// ---------------------------------------------------------
// DRV8871 motor driver pins
// ---------------------------------------------------------
int PIN_MOTOR_A = 18;   // IN1
int PIN_MOTOR_B = 26;   // IN2

// ---------------------------------------------------------
// Limit switches
// ---------------------------------------------------------
int PIN_LIMIT_OPEN  = 33;   // OPEN limit switch
int PIN_LIMIT_CLOSE = 32;   // CLOSE limit switch

// ---------------------------------------------------------
// Manual override switches
// ---------------------------------------------------------
int PIN_SWITCH_OPEN  = 14;
int PIN_SWITCH_CLOSE = 27;