#include "MotorHandler.h"

#include "../../config/Config.h"
#include "../../keyboards/TelegramKeyboards.h"
#include "../../motor/MotorTask.h"
#include "../../motor/MotorModule.h"

namespace MotorHandler {

bool waitingForTimeout = false;
bool waitingForPinch   = false;

// ---------------------------------------------------------
// Helpers
// ---------------------------------------------------------
static bool isNumber(const String& s) {
    if (s.length() == 0) return false;
    for (size_t i = 0; i < s.length(); i++) {
        if (!isDigit(s[i]) && s[i] != '-' && s[i] != '+')
            return false;
    }
    return true;
}

static void sendMotorStatus(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(300);

    out += "🔧 *MOTOR STATUS*\n";
    out += "━━━━━━━━━━━━━━━\n";

    out += "🚪 Door State: ";
    switch (Motor_getState()) {
        case M_OPEN:    out += "OPEN"; break;
        case M_CLOSED:  out += "CLOSED"; break;
        case M_OPENING: out += "OPENING"; break;
        case M_CLOSING: out += "CLOSING"; break;
        case M_STUCK:   out += "STUCK"; break;
        default:        out += "UNKNOWN"; break;
    }
    out += "\n";

    out += "❤️ Health: " + Motor_getHealthString() + "\n";
    out += "🔌 Current: " + String(Motor_getCurrentmA()) + " mA\n";
    out += "⌛ Timeout: " + String(Config_getMotorTimeout()) + " sec\n";
    out += "🐥 Pinch:   " + String(Config_getPinchThreshold()) + " mA\n";
    out += "🌀 Open Cycles:  " + String(Motor_getOpenCycles()) + "\n";
    out += "🌀 Close Cycles: " + String(Motor_getCloseCycles()) + "\n";

    client->sendMessageWithKeyboard(chatId, out, kbMotorMenu());
}

// ---------------------------------------------------------
// Handler
// ---------------------------------------------------------
void handle(const TelegramEvent& evt, TelegramClient* client) {

    // ---------------------------------------------------------
    // BACK during motor input
    // ---------------------------------------------------------
    if (evt.type == EVT_BACK &&
        (waitingForTimeout || waitingForPinch)) {

        waitingForTimeout = false;
        waitingForPinch   = false;

        client->sendMessageWithKeyboard(
            evt.chatId,
            "🔧 *MOTOR MENU*\n━━━━━━━━━━━━━━━",
            kbMotorMenu()
        );
        return;
    }

    // ---------------------------------------------------------
    // Show Motor Menu
    // ---------------------------------------------------------
    if (evt.type == EVT_SHOW_MOTOR_MENU) {
        client->sendMessageWithKeyboard(
            evt.chatId,
            "🔧 *MOTOR MENU*\n━━━━━━━━━━━━━━━\nView status, run tests, or adjust safety settings.",
            kbMotorMenu()
        );
        return;
    }

    // ---------------------------------------------------------
    // Motor Status
    // ---------------------------------------------------------
    if (evt.type == EVT_MOTOR_STATUS) {
        sendMotorStatus(evt.chatId, client);
        return;
    }

    // ---------------------------------------------------------
    // Motor Test (simple open/close pulse)
    // ---------------------------------------------------------
    if (evt.type == EVT_MOTOR_TEST) {
        client->sendMessageWithKeyboard(
            evt.chatId,
            "🔁 Running motor test…",
            kbMotorMenu()
        );

        Motor_requestOpen();
        vTaskDelay(pdMS_TO_TICKS(1000));
        Motor_stop();
        vTaskDelay(pdMS_TO_TICKS(500));
        Motor_requestClose();
        vTaskDelay(pdMS_TO_TICKS(1000));
        Motor_stop();

        return;
    }

    // ---------------------------------------------------------
    // Stop Motor
    // ---------------------------------------------------------
    if (evt.type == EVT_MOTOR_STOP) {
        Motor_stop();
        client->sendMessageWithKeyboard(
            evt.chatId,
            "🛑 Motor stop requested.",
            kbMotorMenu()
        );
        return;
    }

    // ---------------------------------------------------------
    // Reset Health
    // ---------------------------------------------------------
    if (evt.type == EVT_MOTOR_RESET_HEALTH) {
        Motor_setHealth(MOTOR_HEALTH_OK);
        client->sendMessageWithKeyboard(
            evt.chatId,
            "🔄 Motor health reset to OK.",
            kbMotorMenu()
        );
        return;
    }

    // ---------------------------------------------------------
    // Force Stuck
    // ---------------------------------------------------------
    if (evt.type == EVT_MOTOR_FORCE_STUCK) {
        Motor_forceStuck();
        Motor_setHealth(MOTOR_HEALTH_STALLED);
        client->sendMessageWithKeyboard(
            evt.chatId,
            "⚠️ Motor state forced to STUCK.",
            kbMotorMenu()
        );
        return;
    }

    // ---------------------------------------------------------
    // Start Motor Timeout entry
    // ---------------------------------------------------------
    if (evt.type == EVT_SET_MOTOR_TIMEOUT) {
        waitingForTimeout = true;
        waitingForPinch   = false;

        client->sendMessageWithKeyboard(
            evt.chatId,
            "⌛ *Set Motor Timeout*\n\nEnter a number between 10 and 120 seconds.",
            kbMotorMenu()
        );
        return;
    }

    // ---------------------------------------------------------
    // Start Pinch Threshold entry
    // ---------------------------------------------------------
    if (evt.type == EVT_SET_PINCH_THRESHOLD) {
        waitingForTimeout = false;
        waitingForPinch   = true;

        client->sendMessageWithKeyboard(
            evt.chatId,
            "🐥 *Set Pinch Threshold*\n\nEnter a number between 50 and 500 mA.",
            kbMotorMenu()
        );
        return;
    }

    // ---------------------------------------------------------
    // User typed a number (timeout or pinch)
    // ---------------------------------------------------------
    if ((evt.type == EVT_MOTOR_VALUE || evt.type == EVT_OFFSET_VALUE) &&
        (waitingForTimeout || waitingForPinch)) {

        String t = evt.text;
        t.trim();

        if (!isNumber(t)) {
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⛔ Please enter a valid number.",
                kbMotorMenu()
            );
            return;
        }

        int val = t.toInt();

        // Validate ranges
        if (waitingForTimeout && (val < 10 || val > 120)) {
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⛔ Timeout must be between 10 and 120 seconds.",
                kbMotorMenu()
            );
            return;
        }

        if (waitingForPinch && (val < 50 || val > 500)) {
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⛔ Pinch threshold must be between 50 and 500 mA.",
                kbMotorMenu()
            );
            return;
        }

        // Save values
        if (waitingForTimeout) {
            Config_setMotorTimeout(val);
        } else if (waitingForPinch) {
            Config_setPinchThreshold(val);
        }

        Config_save();
        waitingForTimeout = false;
        waitingForPinch   = false;

        String out = "✔ Motor settings updated.\n";
        out += "⌛ Timeout: " + String(Config_getMotorTimeout()) + " sec\n";
        out += "🐥 Pinch:   " + String(Config_getPinchThreshold()) + " mA";

        client->sendMessageWithKeyboard(evt.chatId, out, kbMotorMenu());
        return;
    }

    // ---------------------------------------------------------
    // Unknown motor command
    // ---------------------------------------------------------
    client->sendMessageWithKeyboard(
        evt.chatId,
        "❓ Unknown motor command.",
        kbMotorMenu()
    );
}

} // namespace MotorHandler