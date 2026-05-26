#include "OffsetHandler.h"
#include "../../config/Config.h"
#include "../../keyboards/TelegramKeyboards.h"

// ---------------------------------------------------------
// Static state
// ---------------------------------------------------------
bool OffsetHandler::waitingForOpen  = false;
bool OffsetHandler::waitingForClose = false;

// ---------------------------------------------------------
// Formatting Helper
// ---------------------------------------------------------
String OffsetHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

// ---------------------------------------------------------
// Helpers
// ---------------------------------------------------------
bool OffsetHandler::isInteger(const String& s) {
    if (s.length() == 0) return false;

    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (!isDigit(c) && c != '-' && c != '+')
            return false;
    }
    return true;
}

String OffsetHandler::formatOffset(int val) {
    if (val == 0) return "0 min (exact sunrise/sunset)";
    if (val > 0)  return "+" + String(val) + " min (later)";
    return String(val) + " min (earlier)";
}

// ---------------------------------------------------------
// Handler
// ---------------------------------------------------------
void OffsetHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    // ---------------------------------------------------------
    // BACK during offset entry
    // ---------------------------------------------------------
    if (evt.type == EVT_BACK &&
        (waitingForOpen || waitingForClose))
    {
        waitingForOpen  = false;
        waitingForClose = false;

        String out;
        out.reserve(200);
        out += blockHeader("⚙️", "SETTINGS");
        out += "Adjust system configuration below.";

        client->sendMessageWithKeyboard(evt.chatId, out, kbSettings());
        return;
    }

    // ---------------------------------------------------------
    // Start Open Offset entry
    // ---------------------------------------------------------
    if (evt.type == EVT_SET_OPEN_OFFSET) {

        waitingForOpen  = true;
        waitingForClose = false;

        int current = Config_getOpenOffset();

        String out;
        out.reserve(400);
        out += blockHeader("🌅", "SET OPEN OFFSET");
        out += "Current: *" + formatOffset(current) + "*\n\n";
        out += "Enter a number between *-180* and *+180* minutes.\n";
        out += "Examples:\n";
        out += "• `-30` → open 30 min *before* sunrise\n";
        out += "• `+20` → open 20 min *after* sunrise\n";
        out += "• `0`   → open exactly at sunrise";

        client->sendMessageWithKeyboard(evt.chatId, out, kbSettings());
        return;
    }

    // ---------------------------------------------------------
    // Start Close Offset entry
    // ---------------------------------------------------------
    if (evt.type == EVT_SET_CLOSE_OFFSET) {

        waitingForOpen  = false;
        waitingForClose = true;

        int current = Config_getCloseOffset();

        String out;
        out.reserve(400);
        out += blockHeader("🌇", "SET CLOSE OFFSET");
        out += "Current: *" + formatOffset(current) + "*\n\n";
        out += "Enter a number between *-180* and *+180* minutes.\n";
        out += "Examples:\n";
        out += "• `-30` → close 30 min *before* sunset\n";
        out += "• `+20` → close 20 min *after* sunset\n";
        out += "• `0`   → close exactly at sunset";

        client->sendMessageWithKeyboard(evt.chatId, out, kbSettings());
        return;
    }

    // ---------------------------------------------------------
    // User typed a number
    // ---------------------------------------------------------
    if (evt.type == EVT_OFFSET_VALUE &&
        (waitingForOpen || waitingForClose))
    {
        String t = evt.text;
        t.trim();

        // Empty?
        if (t.length() == 0) {
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⛔ Offset cannot be empty.\nPlease enter a number between -180 and +180.",
                kbSettings()
            );
            return;
        }

        // Must be integer
        if (!isInteger(t)) {
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⛔ Invalid format.\nOnly whole numbers allowed (e.g., -30, 0, +15).",
                kbSettings()
            );
            return;
        }

        int val = t.toInt();

        // Range check
        if (val < -180 || val > 180) {
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⛔ Value out of range.\nMust be between -180 and +180 minutes.",
                kbSettings()
            );
            return;
        }

        // Save
        if (waitingForOpen) {
            Config_setOpenOffset(val);
        } else if (waitingForClose) {
            Config_setCloseOffset(val);
        }

        Config_save();

        waitingForOpen  = false;
        waitingForClose = false;

        // Confirmation message
        String out;
        out.reserve(300);
        out += blockHeader("✔", "OFFSET UPDATED");
        out += "🌅 Open Offset:  " + formatOffset(Config_getOpenOffset()) + "\n";
        out += "🌇 Close Offset: " + formatOffset(Config_getCloseOffset()) + "\n\n";
        out += "Use the menu to adjust other settings.";

        client->sendMessageWithKeyboard(evt.chatId, out, kbSettings());
        return;
    }

    // ---------------------------------------------------------
    // Unknown offset command
    // ---------------------------------------------------------
    client->sendMessageWithKeyboard(
        evt.chatId,
        "❓ Unknown offset command.",
        kbSettings()
    );
}
