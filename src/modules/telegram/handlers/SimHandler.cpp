#include "SimHandler.h"
#include "modules/config/Config.h"
#include "modules/motor/MotorTask.h"
#include "modules/system/Logging.h"
#include <Arduino.h>

namespace SimHandler {

void handle(const TelegramEvent& evt, TelegramClient* client)
{
    const String& txt = evt.text;

    // ---------------------------------------------------------
    // /sim on
    // ---------------------------------------------------------
    if (txt == "/sim on") {
        Config_setSimulatedHardware(true);
        client->sendMessage(evt.chatId, "🧪 Simulation mode ENABLED");
        addLog("SIM → Enabled");
        return;
    }

    // ---------------------------------------------------------
    // /sim off
    // ---------------------------------------------------------
    if (txt == "/sim off") {
        Config_setSimulatedHardware(false);
        client->sendMessage(evt.chatId, "🧪 Simulation mode DISABLED");
        addLog("SIM → Disabled");
        return;
    }

    // ---------------------------------------------------------
    // /sim status
    // ---------------------------------------------------------
    if (txt == "/sim status") {
        bool sim = Config_isSimulatedHardware();
        client->sendMessage(
            evt.chatId,
            String("🧪 Simulation mode is ") + (sim ? "ON" : "OFF")
        );
        return;
    }

    // ---------------------------------------------------------
    // /sim test
    // ---------------------------------------------------------
    if (txt == "/sim test") {
        if (!Config_isSimulatedHardware()) {
            client->sendMessage(
                evt.chatId,
                "❌ Simulation mode is OFF.\nEnable with /sim on"
            );
            return;
        }

        client->sendMessage(evt.chatId, "🧪 Running simulated open/close cycle…");

        Motor_requestOpen();
        addLog("SIM → Test: Opening");
        vTaskDelay(pdMS_TO_TICKS(2500));

        Motor_requestClose();
        addLog("SIM → Test: Closing");

        client->sendMessage(evt.chatId, "🧪 Simulated cycle complete");
        return;
    }

    // ---------------------------------------------------------
    // /sim jam
    // ---------------------------------------------------------
    if (txt == "/sim jam") {
        if (!Config_isSimulatedHardware()) {
            client->sendMessage(
                evt.chatId,
                "❌ Simulation mode is OFF.\nEnable with /sim on"
            );
            return;
        }

        client->sendMessage(evt.chatId, "🧪 Simulating JAMMED door…");

        // NEW: clean public API call
        Motor_forceStuck();
        addLog("SIM → Door forced to STUCK");

        return;
    }
}

} // namespace SimHandler