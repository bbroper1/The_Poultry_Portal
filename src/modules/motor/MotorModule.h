#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Shared motor door state enum (required by many modules)
// ---------------------------------------------------------
enum MotorDoorState {
    M_OPEN,
    M_CLOSED,
    M_OPENING,
    M_CLOSING,
    M_STUCK
};

// ---------------------------------------------------------
// Motor command queue structure
// ---------------------------------------------------------
enum MotorCommandType {
    MOTOR_CMD_OPEN,
    MOTOR_CMD_CLOSE,
    MOTOR_CMD_STOP
};

struct MotorCommand {
    MotorCommandType type;
    uint32_t timestampMs;
};

// ---------------------------------------------------------
// Travel Time / Motion Telemetry
// ---------------------------------------------------------
float  Motor_getAverageTravelTime();
int    Motor_getTravelSampleCount();
time_t Motor_getLastMotionEnd();
time_t Motor_getLastStallTime();
MotorDoorState Motor_getLastStallDirection();

// ---------------------------------------------------------
// Duty Cycle (24h)
// ---------------------------------------------------------
uint32_t Motor_getRuntimeMs24h();
float    Motor_getDutyCycle24h();

// ---------------------------------------------------------
// Motor Health / Diagnostic State
// ---------------------------------------------------------
enum MotorHealth {
    MOTOR_HEALTH_UNKNOWN = 0,
    MOTOR_HEALTH_OK,
    MOTOR_HEALTH_FAULT,
    MOTOR_HEALTH_STALLED,
    MOTOR_HEALTH_OVERCURRENT
};

String      Motor_getHealthString();
MotorHealth Motor_getHealth();
void        Motor_setHealth(MotorHealth health);

// ---------------------------------------------------------
// Motor Current (mA)
// ---------------------------------------------------------
int Motor_getCurrentmA();
