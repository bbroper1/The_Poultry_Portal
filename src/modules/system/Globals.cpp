#include "Globals.h"

// ---------------------------------------------------------
// System-wide state
// ---------------------------------------------------------

unsigned long bootTime = 0;

MenuState menuState = MENU_MAIN;

bool remoteOverride        = false;
bool waitingForOpenOffset  = false;
bool waitingForCloseOffset = false;
bool debugMenuEnabled      = false;
bool factoryResetRequested = false;