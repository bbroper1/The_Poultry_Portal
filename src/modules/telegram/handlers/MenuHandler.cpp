#include "MenuHandler.h"
#include "../../keyboards/TelegramKeyboards.h"

// ---------------------------------------------------------
// Formatting Helper
// ---------------------------------------------------------
String MenuHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

// ---------------------------------------------------------
// Handler
// ---------------------------------------------------------
void MenuHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    String kbd = kbMain();

    String out;
    out.reserve(200);

    out += blockHeader("📋", "MAIN MENU");
    out += "Choose an option below.";

    client->sendMessageWithKeyboard(evt.chatId, out, kbd);
}
