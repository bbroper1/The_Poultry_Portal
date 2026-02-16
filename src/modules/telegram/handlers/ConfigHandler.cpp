#include "ConfigHandler.h"
#include "modules/config/Config.h"
#include "modules/system/Logging.h"

namespace ConfigHandler {

void handle(const TelegramEvent& evt, TelegramClient* client)
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

        client->sendMessage(evt.chatId,
            "✅ Bot token saved.\nReboot recommended.");
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

        client->sendMessage(evt.chatId,
            "✅ Chat ID saved.\nReboot recommended.");
        return;
    }
}

} // namespace ConfigHandler