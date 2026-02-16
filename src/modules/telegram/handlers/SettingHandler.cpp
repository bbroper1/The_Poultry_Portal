#include "SettingsHandler.h"
#include "../../keyboards/TelegramKeyboards.h"

// If you want to log settings access:
// #include "Logging.h"

void SettingsHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    // Settings keyboard (your existing kbSettings())
    String kbd = kbSettings();

    // Optional logging
    // addLog("Menu → Settings opened");

    String msg =
        "⚙️ *SETTINGS*\n"
        "━━━━━━━━━━━━━━━\n"
        "Adjust system configuration below.";

    client->sendMessageWithKeyboard(
        evt.chatId,
        msg,
        kbd
    );
}