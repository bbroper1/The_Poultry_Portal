#pragma once
#include <Arduino.h>
#include <time.h>

// ---------------------------------------------------------
// System-wide state (UI + control flags)
// ---------------------------------------------------------

// Boot timestamp (millis at startup)
extern unsigned long bootTime;

// UI menu state
enum MenuState {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_DEBUG,
    MENU_TIMEZONE,
    MENU_MOTOR
};

extern MenuState menuState;

// Offset input flags (used by Telegram handlers)
extern bool waitingForOpenOffset;
extern bool waitingForCloseOffset;

// Debug menu visibility
extern bool debugMenuEnabled;

// Factory reset request flag
extern bool factoryResetRequested;

// ---------------------------------------------------------
// Manual override system (AutoMode override)
// ---------------------------------------------------------
extern bool  remoteOverride;        // true = manual override active
extern time_t remoteOverrideUntil;  // epoch time when override expires
extern bool  overrideIsOpenCommand; // true=open, false=close

// Optional: initialize globals at boot
void Globals_begin();
