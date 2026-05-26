#include "DoorController.h"
#include "modules/door/DoorModule.h"
#include "modules/motor/MotorTask.h"
#include "modules/config/Config.h"
#include "modules/system/Logging.h"

namespace DoorController {

// ---------------------------------------------------------
// OPEN DOOR
// ---------------------------------------------------------
void openDoor() {
    MotorDoorState st = Motor_getState();

    if (st == M_OPEN || st == M_OPENING) {
        addLog("DoorController → Already open/opening");
        return;
    }

    addLog("DoorController → Opening door");
    Door_setState(DOOR_MOVING);
    Motor_requestOpen();
}

// ---------------------------------------------------------
// CLOSE DOOR
// ---------------------------------------------------------
void closeDoor() {
    MotorDoorState st = Motor_getState();

    if (st == M_CLOSED || st == M_CLOSING) {
        addLog("DoorController → Already closed/closing");
        return;
    }

    addLog("DoorController → Closing door");
    Door_setState(DOOR_MOVING);
    Motor_requestClose();
}

// ---------------------------------------------------------
// AUTO MODE
// ---------------------------------------------------------
void enableAutoMode(bool enabled) {
    addLog(String("DoorController → AutoMode = ") + (enabled ? "ON" : "OFF"));
    Config_setAutoMode(enabled);
}

bool isAutoMode() {
    return Config_getAutoMode();
}

} // namespace DoorController
