#include "DoorModule.h"
#include "modules/system/Logging.h"

// Module-private state
static DoorState s_doorState = DOOR_UNKNOWN;
static time_t s_lastChange = 0;

// ---------------------------------------------------------
// Get raw state
// ---------------------------------------------------------
DoorState Door_getState() {
    return s_doorState;
}

// ---------------------------------------------------------
// Set state
// ---------------------------------------------------------
void Door_setState(DoorState state) {
    if (state == s_doorState)
        return; // no change

    s_doorState = state;
    s_lastChange = time(nullptr);

    addLog(String("Door → State = ") + Door_getStateString());
}

// ---------------------------------------------------------
// Human-readable string
// ---------------------------------------------------------
String Door_getStateString() {
    switch (s_doorState) {
        case DOOR_OPEN:     return "OPEN";
        case DOOR_CLOSED:   return "CLOSED";
        case DOOR_MOVING:   return "MOVING";
        case DOOR_ERROR:    return "ERROR";
        default:            return "UNKNOWN";
    }
}

// ---------------------------------------------------------
// Timestamp of last change
// ---------------------------------------------------------
time_t Door_getLastChangeTime() {
    return s_lastChange;
}

// ---------------------------------------------------------
// Convenience helpers
// ---------------------------------------------------------
bool Door_isOpen()    { return s_doorState == DOOR_OPEN; }
bool Door_isClosed()  { return s_doorState == DOOR_CLOSED; }
bool Door_isMoving()  { return s_doorState == DOOR_MOVING; }
bool Door_isError()   { return s_doorState == DOOR_ERROR; }
