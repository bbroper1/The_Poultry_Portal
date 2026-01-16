#pragma once
#include <Arduino.h>

#define PWM_FREQ 1000
#define PWM_RESOLUTION 8

enum MotorDoorState {
    M_OPEN,
    M_CLOSED,
    M_OPENING,
    M_CLOSING,
    M_STUCK
};

// Lifecycle
void Motor_begin();
void Motor_update();

// High‑level commands
void Motor_requestOpen();
void Motor_requestClose();
void Motor_stop();

// State access
MotorDoorState Motor_getState();

// Diagnostics
float Motor_getCurrent();

// Getters
unsigned int Motor_getOpenCycles();
unsigned int Motor_getCloseCycles();

