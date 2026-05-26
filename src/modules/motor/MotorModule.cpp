#include "MotorModule.h"
#include "modules/battery/BatteryModule.h"   // Battery_getCurrentmA()

// ---------------------------------------------------------
// Module-private state
// ---------------------------------------------------------
static MotorHealth s_motorHealth = MOTOR_HEALTH_UNKNOWN;

// ---------------------------------------------------------
// Motor Health (enum)
// ---------------------------------------------------------
MotorHealth Motor_getHealth() {
    return s_motorHealth;
}

void Motor_setHealth(MotorHealth health) {
    s_motorHealth = health;
}

String Motor_getHealthString() {
    switch (s_motorHealth) {
        case MOTOR_HEALTH_OK:          return "OK";
        case MOTOR_HEALTH_FAULT:       return "FAULT";
        case MOTOR_HEALTH_STALLED:     return "STALLED";
        case MOTOR_HEALTH_OVERCURRENT: return "OVERCURRENT";
        default:                       return "UNKNOWN";
    }
}

// ---------------------------------------------------------
// Motor Current (mA)
// ---------------------------------------------------------
// Currently uses battery current sensor as source.
// Future hardware can replace this with a dedicated motor shunt.
int Motor_getCurrentmA() {
    return Battery_getCurrentmA();
}
