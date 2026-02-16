#include <Arduino.h>
#include "driver/ledc.h"

#include "modules/motor/MotorPins.h"
#include "modules/motor/MotorTask.h"
#include "modules/motor/MotorModule.h"
#include "modules/battery/BatteryModule.h"
#include "modules/config/Config.h"
#include "modules/utils/TimeUtils.h"

// ---------------------------------------------------------
// Globals
// ---------------------------------------------------------
QueueHandle_t g_motorQueue = nullptr;

static TaskHandle_t s_motorTaskHandle = nullptr;

static MotorDoorState s_state = M_CLOSED;
static unsigned int s_openCycles = 0;
static unsigned int s_closeCycles = 0;

static uint32_t s_motionStartMs = 0;
static const uint32_t kSimulatedTravelMs = 2000;

static String lastCommand = "unknown";
static time_t lastMotionStart = 0;

// Telemetry
static int peakCurrent = 0;
static uint32_t currentSum = 0;
static uint32_t currentSamples = 0;

// ---------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------
static void recordCommand(const char* cmd) {
    lastCommand = String(cmd);
    lastMotionStart = time(nullptr);
}

static void motorSetPower(bool forward, uint8_t duty) {
    if (Config_isSimulatedHardware()) {
        Serial.printf("[SIM] Motor %s at duty %d\n",
                      forward ? "OPEN" : "CLOSE", duty);
        return;
    }

    if (forward) {
        ledcWrite(0, duty);
        ledcWrite(1, 0);
    } else {
        ledcWrite(0, 0);
        ledcWrite(1, duty);
    }
}

static void motorStop() {
    if (Config_isSimulatedHardware()) {
        Serial.println("[SIM] Motor STOP");
        return;
    }

    ledcWrite(0, 0);
    ledcWrite(1, 0);
}

// ---------------------------------------------------------
// Motor Task
// ---------------------------------------------------------
static void MotorTask(void *param) {
    MotorCommand cmd;

    for (;;) {

        // Non-blocking receive
        if (xQueueReceive(g_motorQueue, &cmd, pdMS_TO_TICKS(20)) == pdTRUE) {

            switch (cmd.type) {

                case MOTOR_CMD_OPEN:
                    recordCommand("OPEN");
                    s_state = M_OPENING;
                    s_motionStartMs = millis();
                    Motor_setHealth(MOTOR_HEALTH_OK);
                    motorSetPower(true, 200);
                    break;

                case MOTOR_CMD_CLOSE:
                    recordCommand("CLOSE");
                    s_state = M_CLOSING;
                    s_motionStartMs = millis();
                    Motor_setHealth(MOTOR_HEALTH_OK);
                    motorSetPower(false, 200);
                    break;

                case MOTOR_CMD_STOP:
                    recordCommand("STOP");
                    motorStop();
                    if (s_state == M_OPENING) {
                        s_state = M_OPEN;
                        s_openCycles++;
                    } else if (s_state == M_CLOSING) {
                        s_state = M_CLOSED;
                        s_closeCycles++;
                    }
                    break;

                default:
                    break;
            }
        }

        // -----------------------------------------------------
        // SIMULATION MODE
        // -----------------------------------------------------
        if (Config_isSimulatedHardware()) {

            if (s_state == M_OPENING &&
                millis() - s_motionStartMs > kSimulatedTravelMs) {

                s_state = M_OPEN;
                s_openCycles++;
                Serial.println("[SIM] Door reached OPEN");
            }

            if (s_state == M_CLOSING &&
                millis() - s_motionStartMs > kSimulatedTravelMs) {

                s_state = M_CLOSED;
                s_closeCycles++;
                Serial.println("[SIM] Door reached CLOSED");
            }

            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // -----------------------------------------------------
        // REAL HARDWARE: Telemetry
        // -----------------------------------------------------
        if (s_state == M_OPENING || s_state == M_CLOSING) {
            int current = Motor_getCurrentmA();

            // Peak
            if (current > peakCurrent)
                peakCurrent = current;

            // Average
            currentSum += current;
            currentSamples++;
        }

        // -----------------------------------------------------
        // REAL HARDWARE: Limit switches
        // -----------------------------------------------------
        if (s_state == M_OPENING && digitalRead(PIN_LIMIT_OPEN) == LOW) {
            motorStop();
            s_state = M_OPEN;
            s_openCycles++;
            Motor_setHealth(MOTOR_HEALTH_OK);
        }

        if (s_state == M_CLOSING && digitalRead(PIN_LIMIT_CLOSE) == LOW) {
            motorStop();
            s_state = M_CLOSED;
            s_closeCycles++;
            Motor_setHealth(MOTOR_HEALTH_OK);
        }

        // -----------------------------------------------------
        // REAL HARDWARE: Pinch detection
        // -----------------------------------------------------
        if (s_state == M_OPENING || s_state == M_CLOSING) {

            int current = Motor_getCurrentmA();
            int pinch   = Config_getPinchThreshold();

            if (current > pinch) {
                motorStop();
                s_state = M_STUCK;
                Motor_setHealth(MOTOR_HEALTH_OVERCURRENT);

                Serial.printf("[MOTOR] Pinch detected! Current=%dmA Threshold=%dmA\n",
                              current, pinch);
            }
        }

        // -----------------------------------------------------
        // REAL HARDWARE: Timeout
        // -----------------------------------------------------
        if (s_state == M_OPENING || s_state == M_CLOSING) {

            uint32_t timeoutMs = Config_getMotorTimeout() * 1000UL;

            if (millis() - s_motionStartMs > timeoutMs) {
                motorStop();
                s_state = M_STUCK;
                Motor_setHealth(MOTOR_HEALTH_STALLED);

                Serial.printf("[MOTOR] Timeout! Limit=%lu ms\n", timeoutMs);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ---------------------------------------------------------
// Public API
// ---------------------------------------------------------
void Motor_begin() {
    if (!Config_isSimulatedHardware()) {
        pinMode(PIN_MOTOR_A, OUTPUT);
        pinMode(PIN_MOTOR_B, OUTPUT);

        pinMode(PIN_LIMIT_OPEN, INPUT_PULLUP);
        pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);

        ledcSetup(0, PWM_FREQ, PWM_RESOLUTION);
        ledcSetup(1, PWM_FREQ, PWM_RESOLUTION);

        ledcAttachPin(PIN_MOTOR_A, 0);
        ledcAttachPin(PIN_MOTOR_B, 1);
    } else {
        Serial.println("[SIM] Motor hardware disabled");
    }

    g_motorQueue = xQueueCreate(10, sizeof(MotorCommand));

    xTaskCreatePinnedToCore(
        MotorTask,
        "MotorTask",
        4096,
        nullptr,
        2,
        &s_motorTaskHandle,
        1
    );
}

void Motor_requestOpen() {
    MotorCommand cmd{MOTOR_CMD_OPEN, millis()};
    xQueueSend(g_motorQueue, &cmd, 0);
}

void Motor_requestClose() {
    MotorCommand cmd{MOTOR_CMD_CLOSE, millis()};
    xQueueSend(g_motorQueue, &cmd, 0);
}

void Motor_stop() {
    MotorCommand cmd{MOTOR_CMD_STOP, millis()};
    xQueueSend(g_motorQueue, &cmd, 0);
}

void Motor_forceStuck() {
    s_state = M_STUCK;
}

// ---------------------------------------------------------
// Telemetry
// ---------------------------------------------------------
String Motor_getLastCommandString() {
    return lastCommand;
}

String Motor_getLastMotionTimestamp() {
    if (lastMotionStart == 0) return "never";
    return TimeUtils::formatTimestamp(lastMotionStart);
}

int Motor_getPeakCurrentmA() {
    return peakCurrent;
}

int Motor_getAverageCurrentmA() {
    if (currentSamples == 0) return 0;
    return currentSum / currentSamples;
}

// ---------------------------------------------------------
// State access
// ---------------------------------------------------------
MotorDoorState Motor_getState() {
    return s_state;
}

unsigned int Motor_getOpenCycles() {
    return s_openCycles;
}

unsigned int Motor_getCloseCycles() {
    return s_closeCycles;
}