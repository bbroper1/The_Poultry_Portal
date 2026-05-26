#include "SensorTask.h"

#include "modules/sensors/SensorModule.h"
#include "modules/battery/BatteryModule.h"
#include "modules/energy/EnergyModule.h"
#include "modules/system/Logging.h"
#include "modules/config/Config.h"
#include "modules/time/TimeManager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t s_sensorTaskHandle = nullptr;

struct SensorContext {
    TickType_t lastWake = 0;
};

static SensorContext* ctx = nullptr;

static void SensorTask(void* pv) {
    setenv("TZ", Config_getTimezone().c_str(), 1);
    tzset();

    SensorContext* c = ctx;
    const TickType_t interval = pdMS_TO_TICKS(1000);
    c->lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&c->lastWake, interval);

        // -------------------------------------------------
        // 1) Temperature (real sensor read)
        // -------------------------------------------------
        float tempC = Battery_readTemperatureC();  // or your actual sensor
        if (!isnan(tempC)) {
            Sensor_setTemperature(tempC);
        } else {
            addLog("SensorTask → Temp read failed");
        }

        // -------------------------------------------------
        // 2) Battery (INA219)
        // -------------------------------------------------
        Battery_updateFromINA219();

        if (!Battery_isValid()) {
            addLog("SensorTask → INA219 invalid");
            continue;
        }

        float voltage    = Battery_getSmoothedVoltage();
        float current_mA = Battery_getCurrentmA();

        // Brownout detection
        if (Battery_isBrownout()) {
            addLog("SensorTask → BROWNOUT WARNING");
        }

        // -------------------------------------------------
        // 3) System energy (motor energy handled elsewhere)
        // -------------------------------------------------
        float mAh = current_mA * (1.0f / 3600.0f);
        EnergySys_addmAh(mAh);
    }
}

void SensorTask_begin() {
    if (ctx) return;

    ctx = new SensorContext();

    xTaskCreatePinnedToCore(
        SensorTask,
        "SensorTask",
        4096,
        nullptr,
        2,
        &s_sensorTaskHandle,
        1
    );

    addLog("SensorTask → started");
}
