#include "MotorHandler.h"

#include "../../config/Config.h"
#include "../../keyboards/TelegramKeyboards.h"
#include "../../motor/MotorTask.h"
#include "../../motor/MotorModule.h"

// State for numeric input
static bool waitingForTimeout = false;
static bool waitingForPinch   = false;

// ---------------------------------------------------------
// Formatting Helpers
// ---------------------------------------------------------

bool MotorHandler::isNumber(const String& s) {
    if (s.length() == 0) return false;
    for (size_t i = 0; i < s.length(); i++) {
        if (!isDigit(s[i]) && s[i] != '-' && s[i] != '+')
            return false;
    }
    return true;
}

String MotorHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

String MotorHandler::kv(const String& label, const String& value) {
    return "• " + label + ": " + value + "\n";
}

String MotorHandler::formatDoorState(int state) {
    switch (state) {
        case M_OPEN:    return "OPEN";
        case M_CLOSED:  return "CLOSED";
        case M_OPENING: return "OPENING";
        case M_CLOSING: return "CLOSING";
        case M_STUCK:   return "STUCK";
        default:        return "UNKNOWN";
    }
}

// ---------------------------------------------------------
// Motor Status Message
// ---------------------------------------------------------

void MotorHandler::sendMotorStatus(uint64_t chatId, TelegramClient* client) {
    String out;
    out.reserve(400);

    out += blockHeader("🔧", "MOTOR STATUS");
    out += kv("Door State", formatDoorState(Motor_getState()));
    out += kv("Health", Motor_getHealthString());
    out += kv("Current", String(Motor_getCurrentmA()) + " mA");
    out += kv("Timeout", String(Config_getMotorTimeout()) + " sec");
    out += kv("Pinch Threshold", String(Config_getPinchThreshold()) + " mA");
    out += kv("Open Cycles", String(Motor_getOpenCycles()));
    out += kv("Close Cycles", String(Motor_getCloseCycles()));

    client->sendMessageWithKeyboard(chatId, out, kbMotorMenu());
}

// ---------------------------------------------------------
// Main Handler
// ---------------------------------------------------------

void MotorHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    // BACK during motor input
    if (evt.type == EVT_BACK &&
        (waitingForTimeout || waitingForPinch)) {

        waitingForTimeout = false;
        waitingForPinch   = false;

        String out;
        out.reserve(120);
        out += blockHeader("🔧", "MOTOR MENU");
        out += "View status, run tests, or adjust safety settings.";

        client->sendMessageWithKeyboard(evt.chatId, out, kbMotorMenu());
        return;
    }

    // Show Motor Menu
    if (evt.type == EVT_SHOW_MOTOR_MENU) {
        String out;
        out.reserve(160);
        out += blockHeader("🔧", "MOTOR MENU");
        out += "View status, run tests, or adjust safety settings.";

        client->sendMessageWithKeyboard(evt.chatId, out, kbMotorMenu());
        return;
    }

    // Motor Status
    if (evt.type == EVT_MOTOR_STATUS) {
        sendMotorStatus(evt.chatId, client);
        return;
    }

    // Motor Test
    if (evt.type == EVT_MOTOR_TEST) {
        client->sendMessageWithKeyboard(
            evt.chatId,
            "🔁 Running motor test…",
            kbMotorMenu()
        );

        Motor_requestOpen();
        vTaskDelay(pdMS_TO_TICKS(2000));
        Motor_stop();
        vTaskDelay(pdMS_TO_TICKS(1000));
        Motor_requestClose();
        vTaskDelay(pdMS_TO_TICKS(2000));
        Motor_stop();

        return;
    }

    // Stop Motor
    if (evt.type == EVT_MOTOR_STOP) {
        Motor_stop();
        client->sendMessageWithKeyboard(
            evt.chatId,
            "🛑 Motor stop requested.",
            kbMotorMenu()
        );
        return;
    }

    // Reset Health
    if (evt.type == EVT_MOTOR_RESET_HEALTH) {
        Motor_setHealth(MOTOR_HEALTH_OK);
        client->sendMessageWithKeyboard(
            evt.chatId,
            "🔄 Motor health reset to OK.",
            kbMotorMenu()
        );
        return;
    }

    // Force Stuck
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

    // Start Motor Timeout entry
    if (evt.type == EVT_SET_MOTOR_TIMEOUT) {
        waitingForTimeout = true;
        waitingForPinch   = false;

        String out;
        out.reserve(200);
        out += blockHeader("⌛", "SET MOTOR TIMEOUT");
        out += "Enter a number between 10 and 120 seconds.";

        client->sendMessageWithKeyboard(evt.chatId, out, kbMotorMenu());
        return;
    }

    // Start Pinch Threshold entry
    if (evt.type == EVT_SET_PINCH_THRESHOLD) {
        waitingForTimeout = false;
        waitingForPinch   = true;

        String out;
        out.reserve(200);
        out += blockHeader("🐥", "SET PINCH THRESHOLD");
        out += "Enter a number between 50 and 500 mA.";

        client->sendMessageWithKeyboard(evt.chatId, out, kbMotorMenu());
        return;
    }

    // User typed a number (timeout or pinch)
    if (evt.type == EVT_MOTOR_VALUE &&
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

        String out;
        out.reserve(200);
        out += blockHeader("✔", "MOTOR SETTINGS UPDATED");
        out += kv("Timeout", String(Config_getMotorTimeout()) + " sec");
        out += kv("Pinch Threshold", String(Config_getPinchThreshold()) + " mA");

        client->sendMessageWithKeyboard(evt.chatId, out, kbMotorMenu());
        return;
    }

    // Unknown motor command
    client->sendMessageWithKeyboard(
        evt.chatId,
        "❓ Unknown motor command.",
        kbMotorMenu()
    );
}
