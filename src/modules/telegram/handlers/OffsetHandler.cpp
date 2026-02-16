#include "OffsetHandler.h"
#include "../../config/Config.h"
#include "../../keyboards/TelegramKeyboards.h"

namespace OffsetHandler {

bool waitingForOpen  = false;
bool waitingForClose = false;

static bool isNumber(const String& s) {
    if (s.length() == 0) return false;
    for (size_t i = 0; i < s.length(); i++) {
        if (!isDigit(s[i]) && s[i] != '-' && s[i] != '+')
            return false;
    }
    return true;
}

void handle(const TelegramEvent& evt, TelegramClient* client) {

    // ---------------------------------------------------------
    // BACK during offset entry
    // ---------------------------------------------------------
    if (evt.type == EVT_BACK) {
        waitingForOpen  = false;
        waitingForClose = false;

        client->sendMessageWithKeyboard(
            evt.chatId,
            "⚙️ *SETTINGS*\n━━━━━━━━━━━━━━━\nAdjust system configuration below.",
            kbSettings()
        );
        return;
    }

    // ---------------------------------------------------------
    // Start Open Offset entry
    // ---------------------------------------------------------
    if (evt.type == EVT_SET_OPEN_OFFSET) {
        waitingForOpen  = true;
        waitingForClose = false;

        client->sendMessageWithKeyboard(
            evt.chatId,
            "🌅 *Set Open Offset*\n\nEnter a number between -180 and +180 minutes.",
            kbSettings()
        );
        return;
    }

    // ---------------------------------------------------------
    // Start Close Offset entry
    // ---------------------------------------------------------
    if (evt.type == EVT_SET_CLOSE_OFFSET) {
        waitingForOpen  = false;
        waitingForClose = true;

        client->sendMessageWithKeyboard(
            evt.chatId,
            "🌇 *Set Close Offset*\n\nEnter a number between -180 and +180 minutes.",
            kbSettings()
        );
        return;
    }

    // ---------------------------------------------------------
    // User typed a number
    // ---------------------------------------------------------
    if (evt.type == EVT_OFFSET_VALUE &&
        (waitingForOpen || waitingForClose)) {

        String t = evt.text;
        t.trim();

        if (!isNumber(t)) {
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⛔ Please enter a valid number.",
                kbSettings()
            );
            return;
        }

        int val = t.toInt();
        if (val < -180 || val > 180) {
            client->sendMessageWithKeyboard(
                evt.chatId,
                "⛔ Value must be between -180 and +180.",
                kbSettings()
            );
            return;
        }

        if (waitingForOpen) {
            Config_setOpenOffset(val);
        } else if (waitingForClose) {
            Config_setCloseOffset(val);
        }

        Config_save();
        waitingForOpen  = false;
        waitingForClose = false;

        String out = "✔ Offset updated.\n";
        out += "🌅 Open: " + String(Config_getOpenOffset()) + " min\n";
        out += "🌇 Close: " + String(Config_getCloseOffset()) + " min";

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

} // namespace OffsetHandler