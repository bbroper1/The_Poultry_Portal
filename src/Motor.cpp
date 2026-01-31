#include "Motor.h"
#include "HardwarePins.h"
#include "Globals.h"
#include "Config.h"
#include "Battery.h"
#include "Temperature.h"
#include "Logging.h"
#include <WebSerial.h>
#include <Arduino.h>
#include <Adafruit_INA219.h>

// External dependencies
extern float CRITICAL_VOLTAGE;
extern float TEMP_CRITICAL;
extern Adafruit_INA219 ina219;

// -------------------------------
// Internal motor state
// -------------------------------
static MotorDoorState s_state = M_CLOSED;

static unsigned long s_motorStartTime = 0;
static unsigned long s_openLimitTriggeredAt = 0;
static unsigned long s_closeLimitTriggeredAt = 0;
static unsigned int  s_openCycles = 0;
static unsigned int  s_closeCycles = 0;

// -------------------------------
// Public state access
// -------------------------------
unsigned int Motor_getOpenCycles()  { return s_openCycles; }
unsigned int Motor_getCloseCycles() { return s_closeCycles; }

MotorDoorState Motor_getState() {
    return s_state;
}

float Motor_getCurrent() {
    if (!inaOK) return 0.0f;
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
    addLog(testModeActive ? "Motor → SIMULATED OPEN" : "Motor → OPEN request");

    s_state = M_OPENING;
    s_motorStartTime = millis();
    s_openLimitTriggeredAt = 0;

    if (!testModeActive) motorForward();
}

void Motor_requestClose() {
    if (s_state == M_CLOSING || s_state == M_CLOSED) return;
    addLog(testModeActive ? "Motor → SIMULATED CLOSE" : "Motor → CLOSE request");

    s_state = M_CLOSING;
    s_motorStartTime = millis();
    s_closeLimitTriggeredAt = 0;

    if (!testModeActive) motorReverse();
}

void Motor_stop() {
    motorStop();
    if (s_state == M_OPENING)      s_state = M_OPEN;
    else if (s_state == M_CLOSING) s_state = M_CLOSED;
    else                           s_state = M_STUCK;
    addLog("Motor STOP");
}

// -------------------------------
// Main update loop
// -------------------------------
void Motor_update() {
    unsigned long now = millis();

    // 1. TEST MODE
    if (testModeActive) {
        // ... (existing test mode logic)
        return; 
    }

    // 2. READ HARDWARE
    bool openHit  = (digitalRead(PIN_LIMIT_OPEN)  == LOW);
    bool closeHit = (digitalRead(PIN_LIMIT_CLOSE) == LOW);
    float current_mA = Motor_getCurrent();

    // --- TELEMETRY ---
    static unsigned long lastDebug = 0;
    if ((s_state == M_OPENING || s_state == M_CLOSING) && (now - lastDebug > 200)) {
        Serial.printf("[Motor] State:%d | O:%d | C:%d | mA:%.1f\n", s_state, openHit, closeHit, current_mA);
        lastDebug = now;
    }
    if ((s_state == M_OPENING || s_state == M_CLOSING) && (now - lastDebug > 200)) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "[Motor] State:%d | O:%d | C:%d | mA:%.1f", 
                s_state, openHit, closeHit, current_mA);
        
        // Explicitly cast to String to resolve compiler ambiguity
        WebSerial.print(String(buffer) + "\n"); 
        
        WebSerial.println(buffer);
        lastDebug = now;
    }

    // 3. THE MASTER SAFETY KILL-SWITCH
    // If the motor is supposed to be moving, but ANY limit is hit...
    if (s_state == M_OPENING || s_state == M_CLOSING) {
        if (openHit || closeHit) {
            motorStop(); // Kill PWM immediately
            
            // Set the final state based on which way we were going
            if (s_state == M_OPENING) {
                s_state = M_OPEN;
                s_openCycles++;
                addLog("Door OPEN (Limit)");
            } else {
                s_state = M_CLOSED;
                s_closeCycles++;
                addLog("Door CLOSED (Limit)");
            }
            return; // EXIT the function immediately so nothing else restarts the motor
        }
    }

    // 4. SAFETY CHECKS (Battery/Temp)
    if (Battery_getVoltage() < CRITICAL_VOLTAGE || Temperature_getCelsius() > Temperature_getCriticalC()) {
        Motor_stop();
        return;
    }

    // 5. STALL & TIMEOUT PROTECTIONS
    int pinchThreshold = Config_getPinchThreshold();
    int timeoutSec     = Config_getMotorTimeout();

    if (s_state == M_OPENING || s_state == M_CLOSING) {
        
        // Give the motor 200ms to get moving before checking for stalls or limits
        if (now - s_motorStartTime > 200) { 

            // Re-check limits here if you want to be extra safe, 
            // but usually, we just want to ignore stalls at start.
            
            if (current_mA > pinchThreshold) {
                motorStop();
                s_state = M_STUCK;
                addLog("STALL detected");
                return;
            }
        }

        // Timeout Protection (Always active)
        if (now - s_motorStartTime > (unsigned long)timeoutSec * 1000) {
            motorStop();
            s_state = M_STUCK;
            addLog("TIMEOUT reached");
            return;
        }
    }
}
// -------------------------------
// Initialization
// -------------------------------
void Motor_begin() {
    // PWM Config
    ledcSetup(PWM_CH_A, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CH_B, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PIN_MOTOR_A, PWM_CH_A);
    ledcAttachPin(PIN_MOTOR_B, PWM_CH_B);

    // Limit Switch Config - Added INPUT_PULLUP
    pinMode(PIN_LIMIT_OPEN, INPUT_PULLUP);
    pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);

    motorStop();
}