#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Motor health / diagnostic state
// ---------------------------------------------------------
enum MotorHealth {
    MOTOR_HEALTH_UNKNOWN = 0,
    MOTOR_HEALTH_OK,
    MOTOR_HEALTH_FAULT,
    MOTOR_HEALTH_STALLED,
    MOTOR_HEALTH_OVERCURRENT
};

// ---------------------------------------------------------
// Public API
// ---------------------------------------------------------

// Health
String      Motor_getHealthString();
MotorHealth Motor_getHealth();
void        Motor_setHealth(MotorHealth health);

// Motor current (mA)
int Motor_getCurrentmA();