#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Sensor module — cached + smoothed temperature
// ---------------------------------------------------------

// Initialize defaults
void Sensor_begin();

// Set smoothing factor (0.01–1.0)
void Sensor_setSmoothing(float alpha);

// Update cached temperature (called by SensorTask)
void Sensor_setTemperature(float celsius);

// Get last measured temperature in °C
float Sensor_getTemperature();

// Integer version (for display)
int Sensor_getTemperatureInt();

// Timestamp of last update (millis)
time_t Sensor_getLastUpdateTime();

// True if temperature was updated recently
bool Sensor_isFresh(unsigned long maxAgeMs = 5000);
