#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Menu state enum
// ---------------------------------------------------------
enum MenuState {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_DEBUG,
    MENU_TIMEZONE,
    MENU_MOTOR
};

// Global menu state + debug toggle
extern MenuState menuState;
extern bool debugMenuEnabled;

// ---------------------------------------------------------
// Keyboard builders
// ---------------------------------------------------------
String kbMain();
String kbSettings();
String kbDebug();
String kbTimezone();
String kbMotorMenu();

// Internal helper
String wrapKeyboard(const String& rawArray);

//Evnet logs
String kbLogs(int page, int maxPage);