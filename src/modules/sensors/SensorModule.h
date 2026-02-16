#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Sensor module — provides cached temperature
// ---------------------------------------------------------

// Returns the last measured temperature in °C
float Sensor_getTemperature();

// Allows SensorTask to update the cached temperature
void Sensor_setTemperature(float celsius);