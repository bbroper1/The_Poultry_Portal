#pragma once
#include <Arduino.h>

#define VERSION "5.0.0"

// Initialize Preferences and load all settings
void Config_load();

// Save all settings back to flash
void Config_save();

// ----- GETTERS -----
float Config_getLat();
float Config_getLong();
String Config_getTimezone();
String Config_getOTAPassword();
String Config_getTelegramToken();
int Config_getOpenOffset();
int Config_getCloseOffset();
int Config_getMotorTimeout();
int Config_getPinchThreshold();

// ----- SETTERS -----
void Config_setLat(float v);
void Config_setLong(float v);
void Config_setTimezone(const String& tz);
void Config_setOTAPassword(const String& pass);
void Config_setTelegramToken(const String& token);
void Config_setOpenOffset(int v);
void Config_setCloseOffset(int v);
void Config_setMotorTimeout(int v);
void Config_setPinchThreshold(int v);

static const float BATTERY_LOW_VOLTAGE = 11.8f;

