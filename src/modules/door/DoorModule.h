#pragma once
#include <Arduino.h>

// ---------------------------------------------------------
// Door state machine
// ---------------------------------------------------------
enum DoorState {
    DOOR_UNKNOWN = 0,
    DOOR_OPEN,
    DOOR_CLOSED,
    DOOR_MOVING,
    DOOR_ERROR
};

// ---------------------------------------------------------
// Public API
// ---------------------------------------------------------

// Returns a human-readable string for Telegram/status
String Door_getStateString();

// Returns the raw enum (useful for logic)
DoorState Door_getState();

// Allows other modules to update the door state
void Door_setState(DoorState state);