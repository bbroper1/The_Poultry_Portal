#include "MenuHandler.h"
#include "../../keyboards/TelegramKeyboards.h"

// If you want to log menu opens, include Logging.h
// #include "Logging.h"

void MenuHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    // Main menu keyboard
    String kbd = kbMain();

    // Optional: log menu access
    // addLog("Menu → Main menu opened");

    String msg =
        "📋 *MAIN MENU*\n"
        "━━━━━━━━━━━━━━━\n"
        "Choose an option below.";

    client->sendMessageWithKeyboard(
        evt.chatId,
        msg,
        kbd
    );
}