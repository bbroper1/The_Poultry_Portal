#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// ---------------------------------------------------------
// Door State Machine
// ---------------------------------------------------------
enum MotorDoorState {
    M_OPEN,
    M_CLOSED,
    M_OPENING,
    M_CLOSING,
    M_STUCK
};

// ---------------------------------------------------------
// FreeRTOS Motor Command Types
// ---------------------------------------------------------
enum MotorCommandType {
    MOTOR_CMD_NONE = 0,
    MOTOR_CMD_OPEN,
    MOTOR_CMD_CLOSE,
    MOTOR_CMD_STOP
};

struct MotorCommand {
    MotorCommandType type;
    uint64_t requestId;
};

// Global queue handle
extern QueueHandle_t g_motorQueue;

// ---------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------
void Motor_begin();

// ---------------------------------------------------------
// High‑level commands (non‑blocking)
// ---------------------------------------------------------
void Motor_requestOpen();
void Motor_requestClose();
void Motor_stop();
void Motor_forceStuck();

// ---------------------------------------------------------
// State access
// ---------------------------------------------------------
MotorDoorState Motor_getState();

// ---------------------------------------------------------
// Counters
// ---------------------------------------------------------
unsigned int Motor_getOpenCycles();
unsigned int Motor_getCloseCycles();

// ---------------------------------------------------------
// Telemetry
// ---------------------------------------------------------
String Motor_getLastCommandString();
String Motor_getLastMotionTimestamp();
int Motor_getPeakCurrentmA();
int Motor_getAverageCurrentmA();