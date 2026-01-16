#pragma once
#include <Arduino.h>

// Motor pins
extern int PIN_MOTOR_A;
extern int PIN_MOTOR_B;

// Limit switches
extern int PIN_LIMIT_OPEN;
extern int PIN_LIMIT_CLOSE;

// Manual override switches
extern int PIN_SWITCH_OPEN;
extern int PIN_SWITCH_CLOSE;

// Heartbeat LED
extern int PIN_HEARTBEAT;

// PWM channels
extern int PWM_CH_A;
extern int PWM_CH_B;

// Initialization
void HardwarePins_init();
