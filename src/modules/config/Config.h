#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// PoultryPortal Configuration Module
// Loads and saves persistent settings using Preferences.
// ---------------------------------------------------------

#define VERSION "2.1.0"

// ---------------------------------------------------------
// Initialization
// ---------------------------------------------------------
void Config_load();
void Config_save();

// ---------------------------------------------------------
// GETTERS
// ---------------------------------------------------------
float  Config_getLat();
float  Config_getLong();
String Config_getTimezone();
String Config_getOTAPassword();

String Config_getBotToken();
String Config_getChatID();

int Config_getOpenOffset();
int Config_getCloseOffset();
int Config_getMotorTimeout();
int Config_getPinchThreshold();

bool Config_getAutoMode();
bool Config_isSimulatedHardware();

// ---------------------------------------------------------
// SETTERS
// ---------------------------------------------------------
void Config_setLat(float v);
void Config_setLong(float v);
void Config_setTimezone(const String& tz);
void Config_setOTAPassword(const String& pass);

void Config_setBotToken(const String& token);
void Config_setChatID(const String& id);

void Config_setOpenOffset(int v);
void Config_setCloseOffset(int v);
void Config_setMotorTimeout(int v);
void Config_setPinchThreshold(int v);

void Config_setAutoMode(bool enabled);
void Config_setSimulatedHardware(bool enabled);
