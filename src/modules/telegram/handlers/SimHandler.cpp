#include "SimHandler.h"
#include "modules/config/Config.h"
#include "modules/motor/MotorTask.h"
#include "modules/system/Logging.h"
#include <Arduino.h>

// ---------------------------------------------------------
// Formatting Helpers
// ---------------------------------------------------------
String SimHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

String SimHandler::kv(const String& label, const String& value) {
    return "• " + label + ": " + value + "\n";
}

// ---------------------------------------------------------
// Handler
// ---------------------------------------------------------
void SimHandler::handle(const TelegramEvent& evt, TelegramClient* client)
{
    const String& txt = evt.text;

    // ---------------------------------------------------------
    // /sim on
    // ---------------------------------------------------------
    if (txt == "/sim on") {
        Config_setSimulatedHardware(true);
        addLog("SIM → Enabled");

        String out;
        out.reserve(150);
        out += blockHeader("🧪", "SIMULATION MODE");
        out += "Simulation mode *ENABLED*.";

        client->sendMessage(evt.chatId, out);
        return;
    }

    // ---------------------------------------------------------
    // /sim off
    // ---------------------------------------------------------
    if (txt == "/sim off") {
        Config_setSimulatedHardware(false);
        addLog("SIM → Disabled");

        String out;
        out.reserve(150);
        out += blockHeader("🧪", "SIMULATION MODE");
        out += "Simulation mode *DISABLED*.";

        client->sendMessage(evt.chatId, out);
        return;
    }

    // ---------------------------------------------------------
    // /sim status
    // ---------------------------------------------------------
    if (txt == "/sim status") {
        bool sim = Config_isSimulatedHardware();

        String out;
        out.reserve(150);
        out += blockHeader("🧪", "SIMULATION STATUS");
        out += kv("Mode", sim ? "ON" : "OFF");

        client->sendMessage(evt.chatId, out);
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

        Motor_forceStuck();
        addLog("SIM → Door forced to STUCK");

        return;
    }

    // ---------------------------------------------------------
    // Unknown sim command
    // ---------------------------------------------------------
    client->sendMessage(
        evt.chatId,
        "❓ Unknown simulation command."
    );
}
