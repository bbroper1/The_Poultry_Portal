#include "DoorController.h"
#include "modules/door/DoorModule.h"
#include "modules/motor/MotorTask.h"     // <-- Correct motor API
#include "modules/config/Config.h"

namespace {

    // Cache auto mode locally (mirrors Config)
    bool autoMode = false;
}

namespace DoorController {

// ---------------------------------------------------------
// OPEN DOOR
// ---------------------------------------------------------
void openDoor() {

    // Update door state machine
    Door_setState(DOOR_MOVING);

    // Trigger motor
    Motor_requestOpen();
}

// ---------------------------------------------------------
// CLOSE DOOR
// ---------------------------------------------------------
void closeDoor() {

    Door_setState(DOOR_MOVING);

    Motor_requestClose();
}

// ---------------------------------------------------------
// AUTO MODE
// ---------------------------------------------------------
void enableAutoMode(bool enabled) {

    autoMode = enabled;

    // Persist to config
    Config_setAutoMode(enabled);
}

bool isAutoMode() {
    return autoMode;
}

} // namespace DoorController