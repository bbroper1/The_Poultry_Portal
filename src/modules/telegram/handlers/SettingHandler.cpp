#include "SettingsHandler.h"
#include "../../keyboards/TelegramKeyboards.h"

// ---------------------------------------------------------
// Formatting Helper
// ---------------------------------------------------------
String SettingsHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

// ---------------------------------------------------------
// Handler
// ---------------------------------------------------------
void SettingsHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    String out;
    out.reserve(200);

    out += blockHeader("⚙️", "SETTINGS");
    out += "Adjust system configuration below.";

    client->sendMessageWithKeyboard(
        evt.chatId,
        out,
        kbSettings()
    );
}
