#pragma once
#include <Arduino.h>
#include <sunset.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "TimeUtils.h"

// -------------------------------
//  LOGGING / TIME HELPERS
// -------------------------------
bool TimeUtils_timeIsValid();
bool StringUtils_isNumber(const String &s);

// -------------------------------
//  POWER / ENERGY GLOBALS
// -------------------------------
extern float inputVoltage;
extern float outputVoltage;

extern float avgOpenTime;
extern float avgCloseTime;

extern float CRITICAL_VOLTAGE;

// -------------------------------
//  TELEGRAM STATE
// -------------------------------
extern bool telegramEnabled;
extern int64_t userid;

// -------------------------------
//  MENU / MODE STATE
// -------------------------------
enum MenuState {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_DEBUG,
    MENU_TIMEZONE
};
extern MenuState menuState;
extern bool remoteOverride;
extern bool waitingForOpenOffset;
extern bool waitingForCloseOffset;
extern bool debugMenuEnabled;

// -------------------------------
//  PINS
// -------------------------------
extern int PIN_LIMIT_OPEN;
extern int PIN_LIMIT_CLOSE;
extern int PIN_SWITCH_OPEN;
extern int PIN_SWITCH_CLOSE;
extern int PIN_MOTOR_A;
extern int PIN_MOTOR_B;

// -------------------------------
//  SUN POSITION
// -------------------------------
extern SunSet sun;

// -------------------------------
//  BOOT STATE
// -------------------------------
extern unsigned long bootTime;

extern bool inaOK;
extern float motorCurrent;
extern bool factoryResetRequested;

extern WiFiClientSecure secured_client;
extern UniversalTelegramBot bot;
extern bool testModeActive;