#include "MotorController.h"
#include "modules/config/Config.h"
#include "modules/energy/EnergyModule.h"
#include "modules/utils/TimeUtils.h"

// ---------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------
QueueHandle_t MotorController::s_queue = nullptr;
TaskHandle_t MotorController::s_taskHandle = nullptr;

MotorDoorState MotorController::s_state = M_CLOSED;
unsigned int MotorController::s_openCycles = 0;
unsigned int MotorController::s_closeCycles = 0;

time_t MotorController::s_lastMotionEnd = 0;
time_t MotorController::s_lastStallTime = 0;
MotorDoorState MotorController::s_lastStallDirection = M_STUCK;

String MotorController::s_lastCommand = "unknown";
time_t MotorController::s_lastMotionStart = 0;
uint32_t MotorController::s_motionStartMs = 0;

int MotorController::s_peakCurrent = 0;
uint32_t MotorController::s_currentSum = 0;
uint32_t MotorController::s_currentSamples = 0;

float MotorController::s_travelTimes[10] = {0};
int MotorController::s_travelIndex = 0;
int MotorController::s_travelCount = 0;

uint32_t MotorController::s_runtimeMs24h = 0;
time_t MotorController::s_lastRuntimeUpdate = 0;

// ---------------------------------------------------------
// Public API
// ---------------------------------------------------------
void MotorController::begin() {
    if (!Config_isSimulatedHardware()) {
        pinMode(PIN_MOTOR_A, OUTPUT);
        pinMode(PIN_MOTOR_B, OUTPUT);

        pinMode(PIN_LIMIT_OPEN, INPUT_PULLUP);
        pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);

        pinMode(PIN_SWITCH_OPEN, INPUT_PULLUP);
        pinMode(PIN_SWITCH_CLOSE, INPUT_PULLUP);

        digitalWrite(PIN_MOTOR_A, LOW);
        digitalWrite(PIN_MOTOR_B, HIGH);
    }

    s_queue = xQueueCreate(10, sizeof(MotorCommand));

    xTaskCreatePinnedToCore(
        MotorController::taskLoop,
        "MotorTask",
        4096,
        nullptr,
        2,
        &s_taskHandle,
        1
    );
}

void MotorController::requestOpen() {
    MotorCommand cmd{MOTOR_CMD_OPEN, millis()};
    xQueueSend(s_queue, &cmd, 0);
}

void MotorController::requestClose() {
    MotorCommand cmd{MOTOR_CMD_CLOSE, millis()};
    xQueueSend(s_queue, &cmd, 0);
}

void MotorController::stop() {
    MotorCommand cmd{MOTOR_CMD_STOP, millis()};
    xQueueSend(s_queue, &cmd, 0);
}

void MotorController::forceStuck() {
    s_state = M_STUCK;
}

// ---------------------------------------------------------
// Task Loop
// ---------------------------------------------------------
void MotorController::taskLoop(void* param) {
    for (;;) {
        processQueue();
        update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ---------------------------------------------------------
// Queue Processing
// ---------------------------------------------------------
void MotorController::processQueue() {
    MotorCommand cmd;
    if (xQueueReceive(s_queue, &cmd, 0) != pdTRUE)
        return;

    switch (cmd.type) {
        case MOTOR_CMD_OPEN:
            recordCommand("OPEN");
            s_state = M_OPENING;
            Motor_setHealth(MOTOR_HEALTH_OK);
            if (!Config_isSimulatedHardware()) {
                digitalWrite(PIN_MOTOR_A, LOW);
                digitalWrite(PIN_MOTOR_B, LOW);
            }
            break;

        case MOTOR_CMD_CLOSE:
            recordCommand("CLOSE");
            s_state = M_CLOSING;
            Motor_setHealth(MOTOR_HEALTH_OK);
            if (!Config_isSimulatedHardware()) {
                digitalWrite(PIN_MOTOR_A, HIGH);
                digitalWrite(PIN_MOTOR_B, HIGH);
            }
            break;

        case MOTOR_CMD_STOP:
            recordCommand("STOP");
            if (!Config_isSimulatedHardware()) {
                digitalWrite(PIN_MOTOR_A, LOW);
                digitalWrite(PIN_MOTOR_B, HIGH);
            }
            s_state = (s_state == M_OPENING) ? M_OPEN :
                      (s_state == M_CLOSING) ? M_CLOSED :
                      s_state;
            s_lastMotionEnd = time(nullptr);
            break;
    }
}

// ---------------------------------------------------------
// Update Loop
// ---------------------------------------------------------
void MotorController::update() {
    if (Config_isSimulatedHardware()) {
        updateSimulation();
        return;
    }

    updateHardware();
    updateTelemetry();
    checkLimitSwitches();
    checkPinch();
    checkTimeout();
}

// ---------------------------------------------------------
// Simulation Mode
// ---------------------------------------------------------
void MotorController::updateSimulation() {
    if (s_state == M_OPENING &&
        millis() - s_motionStartMs > SIM_TRAVEL_MS) {

        s_state = M_OPEN;
        s_openCycles++;
        s_lastMotionEnd = time(nullptr);
        recordTravelTime((millis() - s_motionStartMs) / 1000.0f);
    }

    if (s_state == M_CLOSING &&
        millis() - s_motionStartMs > SIM_TRAVEL_MS) {

        s_state = M_CLOSED;
        s_closeCycles++;
        s_lastMotionEnd = time(nullptr);
        recordTravelTime((millis() - s_motionStartMs) / 1000.0f);
    }
}

// ---------------------------------------------------------
// Hardware Update
// ---------------------------------------------------------
void MotorController::updateHardware() {
    int swOpen  = digitalRead(PIN_SWITCH_OPEN);
    int swClose = digitalRead(PIN_SWITCH_CLOSE);

    if (swOpen == LOW && swClose == HIGH &&
        s_state != M_OPENING && s_state != M_OPEN)
        requestOpen();

    if (swClose == LOW && swOpen == HIGH &&
        s_state != M_CLOSING && s_state != M_CLOSED)
        requestClose();
}

// ---------------------------------------------------------
// Telemetry
// ---------------------------------------------------------
void MotorController::updateTelemetry() {
    if (s_state != M_OPENING && s_state != M_CLOSING)
        return;

    accumulateRuntime();

    int current = Motor_getCurrentmA();
    if (current > s_peakCurrent)
        s_peakCurrent = current;

    s_currentSum += current;
    s_currentSamples++;

    float motor_mAh = current * (1.0f / 3600.0f);
    EnergyMotor_addmAh(motor_mAh);
}

// ---------------------------------------------------------
// Limit Switches
// ---------------------------------------------------------
void MotorController::checkLimitSwitches() {
    if (s_state == M_OPENING && digitalRead(PIN_LIMIT_OPEN) == LOW) {
        stop();
        s_state = M_OPEN;
        s_openCycles++;
        s_lastMotionEnd = time(nullptr);
        recordTravelTime((millis() - s_motionStartMs) / 1000.0f);
        Motor_setHealth(MOTOR_HEALTH_OK);
    }

    if (s_state == M_CLOSING && digitalRead(PIN_LIMIT_CLOSE) == LOW) {
        stop();
        s_state = M_CLOSED;
        s_closeCycles++;
        s_lastMotionEnd = time(nullptr);
        recordTravelTime((millis() - s_motionStartMs) / 1000.0f);
        Motor_setHealth(MOTOR_HEALTH_OK);
    }
}

// ---------------------------------------------------------
// Pinch Detection
// ---------------------------------------------------------
void MotorController::checkPinch() {
    if (s_state != M_OPENING && s_state != M_CLOSING)
        return;

    int current = Motor_getCurrentmA();
    int pinch = Config_getPinchThreshold();

    if (current > pinch) {
        stop();
        recordStallEvent();
        s_state = M_STUCK;
        s_lastMotionEnd = time(nullptr);
        Motor_setHealth(MOTOR_HEALTH_OVERCURRENT);
    }
}

// ---------------------------------------------------------
// Timeout
// ---------------------------------------------------------
void MotorController::checkTimeout() {
    if (s_state != M_OPENING && s_state != M_CLOSING)
        return;

    uint32_t timeoutMs = Config_getMotorTimeout() * 1000UL;

    if (millis() - s_motionStartMs > timeoutMs) {
        stop();
        recordStallEvent();
        s_state = M_STUCK;
        s_lastMotionEnd = time(nullptr);
        Motor_setHealth(MOTOR_HEALTH_STALLED);
    }
}

// ---------------------------------------------------------
// Helpers
// ---------------------------------------------------------
void MotorController::recordCommand(const char* cmd) {
    s_lastCommand = cmd;
    s_lastMotionStart = time(nullptr);
    s_motionStartMs = millis();
}

void MotorController::recordTravelTime(float seconds) {
    s_travelTimes[s_travelIndex] = seconds;
    s_travelIndex = (s_travelIndex + 1) % 10;
    if (s_travelCount < 10)
        s_travelCount++;
}

void MotorController::recordStallEvent() {
    s_lastStallTime = time(nullptr);
    s_lastStallDirection = s_state;
}

void MotorController::accumulateRuntime() {
    time_t now = time(nullptr);

    if (s_lastRuntimeUpdate == 0) {
        s_lastRuntimeUpdate = now;
        return;
    }

    uint32_t delta = now - s_lastRuntimeUpdate;
    s_lastRuntimeUpdate = now;

    s_runtimeMs24h += delta * 1000UL;

    const uint32_t DAY_MS = 24UL * 60UL * 60UL * 1000UL;
    if (s_runtimeMs24h > DAY_MS)
        s_runtimeMs24h = DAY_MS;
}

// ---------------------------------------------------------
// Public Getters
// ---------------------------------------------------------
MotorDoorState MotorController::getState() { return s_state; }
unsigned int MotorController::getOpenCycles() { return s_openCycles; }
unsigned int MotorController::getCloseCycles() { return s_closeCycles; }
time_t MotorController::getLastMotionEnd() { return s_lastMotionEnd; }
time_t MotorController::getLastStallTime() { return s_lastStallTime; }
MotorDoorState MotorController::getLastStallDirection() { return s_lastStallDirection; }

String MotorController::getLastCommandString() { return s_lastCommand; }
String MotorController::getLastMotionTimestamp() {
    if (s_lastMotionStart == 0) return "never";
    return TimeUtils::formatTimestamp(s_lastMotionStart);
}

int MotorController::getPeakCurrentmA() { return s_peakCurrent; }
int MotorController::getAverageCurrentmA() {
    if (s_currentSamples == 0) return 0;
    return s_currentSum / s_currentSamples;
}

float MotorController::getAverageTravelTime() {
    if (s_travelCount == 0) return 0;
    float sum = 0;
    for (int i = 0; i < s_travelCount; i++)
        sum += s_travelTimes[i];
    return sum / s_travelCount;
}

int MotorController::getTravelSampleCount() {
    return s_travelCount;
}

uint32_t MotorController::getRuntimeMs24h() { return s_runtimeMs24h; }
float MotorController::getDutyCycle24h() {
    const float DAY_MS = 24.0f * 60.0f * 60.0f * 1000.0f;
    return (float)s_runtimeMs24h / DAY_MS * 100.0f;
}
