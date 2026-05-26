#include "Globals.h"

// ---------------------------------------------------------
// System-wide state
// ---------------------------------------------------------

unsigned long bootTime = 0;

MenuState menuState = MENU_MAIN;

bool waitingForOpenOffset  = false;
bool waitingForCloseOffset = false;
bool debugMenuEnabled      = false;
bool factoryResetRequested = false;

// ---------------------------------------------------------
// Manual override system
// ---------------------------------------------------------
bool  remoteOverride        = false;
time_t remoteOverrideUntil  = 0;
bool  overrideIsOpenCommand = false;

// ---------------------------------------------------------
// Initialization helper
// ---------------------------------------------------------
void Globals_begin() {
    bootTime = millis();

    menuState = MENU_MAIN;

    waitingForOpenOffset  = false;
    waitingForCloseOffset = false;
    debugMenuEnabled      = false;
    factoryResetRequested = false;

    remoteOverride        = false;
    remoteOverrideUntil   = 0;
    overrideIsOpenCommand = false;
}
