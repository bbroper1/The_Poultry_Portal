#pragma once
#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include "Globals.h"

// Forward declarations for keyboard builders
String kbMain();
String kbSettings();
String kbDebug();

// Router entry points
void TelegramRouter_init();
void TelegramRouter_handle();

// Global bot instance (defined in Globals.cpp)
extern UniversalTelegramBot bot;
