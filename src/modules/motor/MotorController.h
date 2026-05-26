#pragma once
#include <Arduino.h>
#include "MotorPins.h"
#include "MotorModule.h"

class MotorController {
public:
    // -----------------------------------------------------
    // Initialization + Commands
    // -----------------------------------------------------
    static void begin();
    static void requestOpen();
    static void requestClose();
    static void stop();
    static void forceStuck();

    // -----------------------------------------------------
    // State access
    // -----------------------------------------------------
    static MotorDoorState getState();
    static unsigned int getOpenCycles();
    static unsigned int getCloseCycles();
    static time_t getLastMotionEnd();
    static time_t getLastStallTime();
    static MotorDoorState getLastStallDirection();

    // -----------------------------------------------------
    // Telemetry
    // -----------------------------------------------------
    static String getLastCommandString();
    static String getLastMotionTimestamp();
    static int getPeakCurrentmA();
    static int getAverageCurrentmA();
    static float getAverageTravelTime();
    static int getTravelSampleCount();
    static uint32_t getRuntimeMs24h();
    static float getDutyCycle24h();

private:
    // -----------------------------------------------------
    // Task + Update Loop
    // -----------------------------------------------------
    static void taskLoop(void* param);
    static void processQueue();
    static void update();
    static void updateSimulation();
    static void updateHardware();
    static void updateTelemetry();
    static void checkLimitSwitches();
    static void checkPinch();
    static void checkTimeout();

    // -----------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------
    static void recordTravelTime(float seconds);
    static void recordStallEvent();
    static void recordCommand(const char* cmd);
    static void accumulateRuntime();

    // -----------------------------------------------------
    // Internal state
    // -----------------------------------------------------
    static MotorDoorState s_state;
    static unsigned int s_openCycles;
    static unsigned int s_closeCycles;

    static time_t s_lastMotionEnd;
    static time_t s_lastStallTime;
    static MotorDoorState s_lastStallDirection;

    static String s_lastCommand;
    static time_t s_lastMotionStart;
    static uint32_t s_motionStartMs;

    // -----------------------------------------------------
    // Telemetry
    // -----------------------------------------------------
    static int s_peakCurrent;
    static uint32_t s_currentSum;
    static uint32_t s_currentSamples;

    // -----------------------------------------------------
    // Travel time buffer (10 samples)
    // -----------------------------------------------------
    static float s_travelTimes[10];
    static int s_travelIndex;
    static int s_travelCount;

    // -----------------------------------------------------
    // Duty cycle (24h)
    // -----------------------------------------------------
    static uint32_t s_runtimeMs24h;
    static time_t s_lastRuntimeUpdate;

    // -----------------------------------------------------
    // Queue + Task
    // -----------------------------------------------------
    static QueueHandle_t s_queue;
    static TaskHandle_t s_taskHandle;

    // -----------------------------------------------------
    // Simulation constants
    // -----------------------------------------------------
    static const uint32_t SIM_TRAVEL_MS = 2000;
};
