#pragma once

#include <Arduino.h>

namespace DoorController {

    // High-level door actions
    void openDoor();
    void closeDoor();
    void enableAutoMode(bool enabled);

    // Optional: expose state if needed later
    bool isAutoMode();
}