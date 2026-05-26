#pragma once
#include <Arduino.h>

namespace DoorController {

    // High-level door actions
    void openDoor();
    void closeDoor();

    // Auto mode control
    void enableAutoMode(bool enabled);
    bool isAutoMode();
}
