#include "MotorTask.h"
#include "MotorController.h"

// ---------------------------------------------------------
// Initialization
// ---------------------------------------------------------
void Motor_begin() {
    MotorController::begin();
}

// ---------------------------------------------------------
// Commands
// ---------------------------------------------------------
void Motor_requestOpen()  { MotorController::requestOpen(); }
void Motor_requestClose() { MotorController::requestClose(); }
void Motor_stop()         { MotorController::stop(); }
void Motor_forceStuck()   { MotorController::forceStuck(); }

// ---------------------------------------------------------
// State Access
// ---------------------------------------------------------
MotorDoorState Motor_getState()              { return MotorController::getState(); }
unsigned int   Motor_getOpenCycles()         { return MotorController::getOpenCycles(); }
unsigned int   Motor_getCloseCycles()        { return MotorController::getCloseCycles(); }
time_t         Motor_getLastMotionEnd()      { return MotorController::getLastMotionEnd(); }
time_t         Motor_getLastStallTime()      { return MotorController::getLastStallTime(); }
MotorDoorState Motor_getLastStallDirection() { return MotorController::getLastStallDirection(); }

// ---------------------------------------------------------
// Telemetry
// ---------------------------------------------------------
String   Motor_getLastCommandString()   { return MotorController::getLastCommandString(); }
String   Motor_getLastMotionTimestamp() { return MotorController::getLastMotionTimestamp(); }
int      Motor_getPeakCurrentmA()       { return MotorController::getPeakCurrentmA(); }
int      Motor_getAverageCurrentmA()    { return MotorController::getAverageCurrentmA(); }
float    Motor_getAverageTravelTime()   { return MotorController::getAverageTravelTime(); }
int      Motor_getTravelSampleCount()   { return MotorController::getTravelSampleCount(); }
uint32_t Motor_getRuntimeMs24h()        { return MotorController::getRuntimeMs24h(); }
float    Motor_getDutyCycle24h()        { return MotorController::getDutyCycle24h(); }

TaskHandle_t s_motorTaskHandle = nullptr;
