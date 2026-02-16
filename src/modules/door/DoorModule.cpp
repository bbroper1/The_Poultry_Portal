#include "DoorModule.h"

// Module-private state
static DoorState s_doorState = DOOR_UNKNOWN;

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
    s_doorState = state;
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