#pragma once
#include <Arduino.h>
#include "modules/system/Globals.h"

extern MenuState menuState;
extern bool debugMenuEnabled;

String kbMain();
String kbSettings();
String kbDebug();
String kbTimezone();
String kbMotorMenu();
String kbOverrideMenu(bool isOpen);
String kbLogs(int page, int maxPage);

String wrapKeyboard(const String& rawArray);
