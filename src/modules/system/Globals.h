#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// System-wide state (UI + control flags)
// ---------------------------------------------------------

extern unsigned long bootTime;

// UI menu state
enum MenuState {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_DEBUG,
    MENU_TIMEZONE
};

extern MenuState menuState;

// Control flags
extern bool remoteOverride;
extern bool waitingForOpenOffset;
extern bool waitingForCloseOffset;
extern bool debugMenuEnabled;
extern bool factoryResetRequested;