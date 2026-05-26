#pragma once
#include <Arduino.h>

// Initializes all non-motor hardware pins
void HardwarePins_begin();

// Heartbeat helpers
void HardwarePins_heartbeatOn();
void HardwarePins_heartbeatOff();
void HardwarePins_heartbeatToggle();
void HardwarePins_blink(int times, int ms = 100);

// Optional: expose heartbeat pin for diagnostics
int HardwarePins_getHeartbeatPin();
