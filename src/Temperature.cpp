#include "Temperature.h"
#include "Motor.h"
#include "Config.h"
#include "Logging.h"
#include "Globals.h"   // <-- needed for bot + userid

extern int64_t userid;

// ESP32 internal temperature sensor
extern "C" {
    uint8_t temprature_sens_read();
}

// Cached temperature
static float s_tempC = 0.0f;
static bool s_warningShown = false;

// Thresholds (module-private)
static constexpr float TEMP_WARNING_C  = 70.0;   // adjust as needed
static constexpr float TEMP_CRITICAL_C = 80.0;   // adjust as needed

// Threshold getters
float Temperature_getWarningC() {
    return TEMP_WARNING_C;
}

float Temperature_getCriticalC() {
    return TEMP_CRITICAL_C;
}

void Temperature_begin() {
    // Nothing required yet, but this keeps the API consistent
}

float Temperature_getCelsius() {
    return s_tempC;
}

bool Temperature_isCritical() {
    return s_tempC >= TEMP_CRITICAL_C;
}

bool Temperature_isWarning() {
    return s_tempC >= TEMP_WARNING_C;
}

String Temperature_getLabel() {
    if (Temperature_isCritical()) return "🔥 CRITICAL";
    if (Temperature_isWarning())  return "⚠️ HIGH";
    return "OK";
}

void Temperature_update() {
    // Convert raw sensor reading to °C
    s_tempC = (temprature_sens_read() - 32) / 1.8;

    // --- CRITICAL TEMPERATURE ---
    if (Temperature_isCritical()) {
        Motor_stop();

        if (Motor_getState() == M_OPENING || Motor_getState() == M_CLOSING) {
            Motor_stop();
            addLog("OVERHEAT! 🔥");

            if (Config_getBotToken().length() > 0) {
                bot.sendMessage(
                    String(userid),
                    "🔥 *CRITICAL TEMP:* " + String(s_tempC, 1) + "°C\nMotor stopped!",
                    "Markdown"
                );
            }
        }
        return;
    }

    // --- WARNING TEMPERATURE ---
    if (Temperature_isWarning() && !s_warningShown) {
        addLog("High Temp ⚠️");

        if (Config_getBotToken().length() > 0) {
            bot.sendMessage(
                String(userid),
                "⚠️ *High temperature:* " + String(s_tempC, 1) + "°C",
                "Markdown"
            );
        }

        s_warningShown = true;
        return;
    }

    // --- RESET WARNING FLAG ---
    if (s_tempC < Temperature_getWarningC() - 5) {
        s_warningShown = false;
    }
}
