#pragma once
#include <UniversalTelegramBot.h>

typedef telegramMessage TBMessage;

// Global bot instance (defined in Globals.cpp)
extern UniversalTelegramBot bot;
extern String kbTimezone();

// Keyboard builders (defined in TelegramRouter.cpp)
String kbMain();
String kbSettings();
String kbDebug();

// ----- Core Handlers -----
void handleStatus(TBMessage &msg);
void handleEnergy(TBMessage &msg);
void handleLogs(TBMessage &msg);
void handleOpen(TBMessage &msg);
void handleClose(TBMessage &msg);
void handleAuto(TBMessage &msg);
void handleHealth(TBMessage &msg);

// ----- Settings Handlers -----
void handleSettings(TBMessage &msg);
void handleTimezone(TBMessage &msg);
void handleLocation(TBMessage &msg);
void handleOffsets(TBMessage &msg);
void handlePinchThreshold(TBMessage &msg);
void handleMotorTimeout(TBMessage &msg);

// ----- System Actions -----
void handleResetEnergy(TBMessage &msg);
void handleReboot(TBMessage &msg);
void handleHelp(TBMessage &msg);
void handleUnknown(TBMessage &msg);

// ----- Debug Handlers -----
void handleDebugDoor(TBMessage &msg);
void handleDebugTime(TBMessage &msg);
void handleDebugSun(TBMessage &msg);
void handleDebugAuto(TBMessage &msg);
void handleDebugState(TBMessage &msg);
void handleDebugLimits(TBMessage &msg);
void handleDebugConfig(TBMessage &msg);
void handleDebugEnergy(TBMessage &msg);
void handleDebugAll(TBMessage &msg);
void handleDebugMenu(TBMessage &msg);
void handleDebugOn(TBMessage &msg);
void handleDebugOff(TBMessage &msg);
void handleTestModeOn(TBMessage &msg);
void handleTestModeOff(TBMessage &msg);
