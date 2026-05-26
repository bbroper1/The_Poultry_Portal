#pragma once
#include "MotorModule.h"

// Initialization
void Motor_begin();

// Commands
void Motor_requestOpen();
void Motor_requestClose();
void Motor_stop();
void Motor_forceStuck();

// State access
MotorDoorState Motor_getState();
unsigned int   Motor_getOpenCycles();
unsigned int   Motor_getCloseCycles();
time_t         Motor_getLastMotionEnd();
time_t         Motor_getLastStallTime();
MotorDoorState Motor_getLastStallDirection();

// Telemetry
String   Motor_getLastCommandString();
String   Motor_getLastMotionTimestamp();
int      Motor_getPeakCurrentmA();
int      Motor_getAverageCurrentmA();
float    Motor_getAverageTravelTime();
int      Motor_getTravelSampleCount();     // ⭐ missing before
uint32_t Motor_getRuntimeMs24h();
float    Motor_getDutyCycle24h();
