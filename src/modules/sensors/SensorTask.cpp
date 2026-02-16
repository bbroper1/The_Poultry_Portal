#include "SensorTask.h"

#include "modules/sensors/SensorModule.h"
#include "modules/battery/BatteryModule.h"
#include "modules/energy/EnergyModule.h"
#include "modules/system/Logging.h"
#include "modules/config/Config.h"
#include "modules/time/TimeManager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ---------------------------------------------------------
// INTERNAL CONTEXT
// ---------------------------------------------------------
struct SensorContext {
    TickType_t lastWake = 0;
};

static SensorContext* ctx = nullptr;
static TaskHandle_t s_sensorTaskHandle = nullptr;

// ---------------------------------------------------------
// SENSOR TASK
// ---------------------------------------------------------
static void SensorTask(void* pv) {
    setenv("TZ", Config_getTimezone().c_str(), 1);
    tzset();
    SensorContext* c = ctx;
    const TickType_t interval = pdMS_TO_TICKS(1000); // 1 second
    c->lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&c->lastWake, interval);

        // -------------------------------------------------
        // 1) Temperature update (SensorModule)
        // -------------------------------------------------
        float tempC = Sensor_getTemperature();   // modern API
        Sensor_setTemperature(tempC);

        // -------------------------------------------------
        // 2) Battery update (BatteryModule)
        // -------------------------------------------------
        float voltage = Battery_getVoltage();    // modern API
        Battery_setVoltage(voltage);

        float current_mA = Battery_getCurrentmA();   // modern API

        // -------------------------------------------------
        // 3) Energy accounting (EnergyModule)
        //    Convert current (mA) over 1 second → mAh
        // -------------------------------------------------
        float mAh = current_mA * (1.0f / 3600.0f);
        Energy_addmAh(mAh);
        Energy_update();

        // -------------------------------------------------
        // 4) SystemStatus heartbeat (REMOVED)
        // -------------------------------------------------
        // SystemStatus_updateHeartbeat();   // obsolete — removed
    }
}

// ---------------------------------------------------------
// PUBLIC API
// ---------------------------------------------------------
void SensorTask_begin() {
    if (ctx) return;

    ctx = new SensorContext();

    xTaskCreatePinnedToCore(
        SensorTask,
        "SensorTask",
        4096,
        nullptr,
        2,      // medium-low priority
        &s_sensorTaskHandle,
        1       // run on core 1 with Motor/Auto/Scheduler
    );

    addLog("SensorTask → started");
}