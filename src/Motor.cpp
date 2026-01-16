#include "Motor.h"
#include "HardwarePins.h" 
#include "Globals.h"
#include "Config.h"
#include "Battery.h"
#include "Temperature.h"
#include "Logging.h"
#include <Arduino.h>
#include <AsyncTelegram2.h>
#include <Adafruit_INA219.h>

   
// External dependencies from main.cpp
extern float CRITICAL_VOLTAGE;
extern float Battery_getVoltage();
extern float esp32Temp;
extern float TEMP_CRITICAL;
extern Adafruit_INA219 ina219;

// -------------------------------
// Internal motor state
// -------------------------------
static MotorDoorState s_state = M_CLOSED;

static unsigned long s_motorStartTime = 0;
static unsigned long s_openLimitTriggeredAt = 0;
static unsigned long s_closeLimitTriggeredAt = 0;
static unsigned int s_openCycles = 0;
static unsigned int s_closeCycles = 0;

unsigned int Motor_getOpenCycles() { return s_openCycles; }
unsigned int Motor_getCloseCycles() { return s_closeCycles; }


// -------------------------------
// Public state access
// -------------------------------
MotorDoorState Motor_getState() {
    return s_state;
}

float Motor_getCurrent() { 
    return ina219.getCurrent_mA(); 
}

// -------------------------------
// Motor control helpers
// -------------------------------
static void motorForward() {
    ledcWrite(PWM_CH_A, 255);
    ledcWrite(PWM_CH_B, 0);
}

static void motorReverse() {
    ledcWrite(PWM_CH_A, 0);
    ledcWrite(PWM_CH_B, 255);
}

static void motorStop() {
    ledcWrite(PWM_CH_A, 0);
    ledcWrite(PWM_CH_B, 0);
}

// -------------------------------
// Public API
// -------------------------------
void Motor_requestOpen() {
    if (s_state == M_OPENING || s_state == M_OPEN) return;

    addLog("Motor → OPEN request");

    s_state = M_OPENING;
    s_motorStartTime = millis();
    s_openLimitTriggeredAt = 0;

    motorForward();
}

void Motor_requestClose() {
    if (s_state == M_CLOSING || s_state == M_CLOSED) return;

    addLog("Motor → CLOSE request");

    s_state = M_CLOSING;
    s_motorStartTime = millis();
    s_closeLimitTriggeredAt = 0;

    motorReverse();
}

void Motor_stop() {
    motorStop();
    if (s_state == M_OPENING) s_state = M_OPEN;
    else if (s_state == M_CLOSING) s_state = M_CLOSED;
    else s_state = M_STUCK;

    addLog("Motor STOP");
}

// -------------------------------
// Main update loop
// -------------------------------
void Motor_update() {
    unsigned long now = millis();

    if (Battery_getVoltage() < CRITICAL_VOLTAGE) {
    Motor_stop();
    addLog("Motor STOP: low battery");
    return;
    }
    if (Temperature_getCelsius() > Temperature_getCriticalC()) {
        Motor_stop();
        addLog("Motor STOP: high temperature");
        return;
    }


    // Read limit switches
    bool openHit  = (digitalRead(PIN_LIMIT_OPEN)  == LOW);
    bool closeHit = (digitalRead(PIN_LIMIT_CLOSE) == LOW);

    // Read motor current
    float current_mA = Motor_getCurrent();

    // Config values
    int pinchThreshold = Config_getPinchThreshold();
    int timeoutSec     = Config_getMotorTimeout();

    // -------------------------------
    // OPENING
    // -------------------------------
    if (s_state == M_OPENING) {

        // Limit switch hit
        if (openHit) {
            if (s_openLimitTriggeredAt == 0)
                s_openLimitTriggeredAt = now;

            if (now - s_openLimitTriggeredAt > 150) {
                motorStop();
                s_state = M_OPEN;
                addLog("Door OPEN");
                s_openCycles++;
                return;
            }
        }

        // Stall detection
        if (current_mA > pinchThreshold) {
            motorStop();
            s_state = M_STUCK;
            addLog("STALL during OPEN");
            return;
        }

        // Timeout
        if (now - s_motorStartTime > timeoutSec * 1000) {
            motorStop();
            s_state = M_STUCK;
            addLog("TIMEOUT during OPEN");
            return;
        }
    }

    // -------------------------------
    // CLOSING
    // -------------------------------
    if (s_state == M_CLOSING) {

        // Limit switch hit
        if (closeHit) {
            if (s_closeLimitTriggeredAt == 0)
                s_closeLimitTriggeredAt = now;

            if (now - s_closeLimitTriggeredAt > 150) {
                motorStop();
                s_state = M_CLOSED;
                addLog("Door CLOSED");
                s_closeCycles++;
                return;
            }
        }

        // Stall detection
        if (current_mA > pinchThreshold) {
            motorStop();
            s_state = M_STUCK;
            addLog("STALL during CLOSE");
            return;
        }

        // Timeout
        if (now - s_motorStartTime > timeoutSec * 1000) {
            motorStop();
            s_state = M_STUCK;
            addLog("TIMEOUT during CLOSE");
            return;
        }
    }
}
void Motor_begin() {
    ledcSetup(PWM_CH_A, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CH_B, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PIN_MOTOR_A, PWM_CH_A);
    ledcAttachPin(PIN_MOTOR_B, PWM_CH_B);

    motorStop();
}
