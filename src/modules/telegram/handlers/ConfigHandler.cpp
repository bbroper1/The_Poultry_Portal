#include "ConfigHandler.h"
#include "modules/config/Config.h"
#include "modules/system/Logging.h"

// ---------------------------------------------------------
// Formatting Helpers
// ---------------------------------------------------------
String ConfigHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

String ConfigHandler::kv(const String& label, const String& value) {
    return "• " + label + ": " + value + "\n";
}

// ---------------------------------------------------------
// Main Handler
// ---------------------------------------------------------
void ConfigHandler::handle(const TelegramEvent& evt, TelegramClient* client)
{
    const String& txt = evt.text;

    // ---------------------------------------------------------
    // /settoken <token>
    // ---------------------------------------------------------
    if (txt.startsWith("/settoken")) {

        int space = txt.indexOf(' ');
        if (space < 0) {
            client->sendMessage(evt.chatId,
                "❌ Usage:\n/settoken <your_bot_token>");
            return;
        }

        String token = txt.substring(space + 1);
        token.trim();

        if (token.length() < 10) {
            client->sendMessage(evt.chatId,
                "❌ Invalid token. Must be longer.");
            return;
        }

        Config_setBotToken(token);
        addLog("CONFIG → Bot token updated");

        String out;
        out.reserve(200);
        out += blockHeader("🔐", "BOT TOKEN UPDATED");
        out += "New token saved.\nReboot recommended.";

        client->sendMessage(evt.chatId, out);
        return;
    }

    // ---------------------------------------------------------
    // /setchat <chat_id>
    // ---------------------------------------------------------
    if (txt.startsWith("/setchat")) {

        int space = txt.indexOf(' ');
        if (space < 0) {
            client->sendMessage(evt.chatId,
                "❌ Usage:\n/setchat <chat_id>");
            return;
        }

        String chat = txt.substring(space + 1);
        chat.trim();

        if (chat.length() < 5) {
            client->sendMessage(evt.chatId,
                "❌ Invalid chat ID.");
            return;
        }

        Config_setChatID(chat);
        addLog("CONFIG → Chat ID updated");

        String out;
        out.reserve(200);
        out += blockHeader("💬", "CHAT ID UPDATED");
        out += kv("Chat ID", chat);
        out += "\nReboot recommended.";

        client->sendMessage(evt.chatId, out);
        return;
    }

    // ---------------------------------------------------------
    // Unknown command
    // ---------------------------------------------------------
    client->sendMessage(
        evt.chatId,
        "❓ Unknown config command."
    );
}
