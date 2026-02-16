#include "MotorModule.h"
#include "modules/battery/BatteryModule.h"   // for Battery_getCurrentmA()

// Module-private state
static MotorHealth s_motorHealth = MOTOR_HEALTH_UNKNOWN;

// ---------------------------------------------------------
// Get raw health enum
// ---------------------------------------------------------
MotorHealth Motor_getHealth() {
    return s_motorHealth;
}

// ---------------------------------------------------------
// Set health enum
// ---------------------------------------------------------
void Motor_setHealth(MotorHealth health) {
    s_motorHealth = health;
}

// ---------------------------------------------------------
// Human-readable string
// ---------------------------------------------------------
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
// Motor current (mA)
// Currently uses battery current sensor as source
// ---------------------------------------------------------
int Motor_getCurrentmA() {
    return Battery_getCurrentmA();
}