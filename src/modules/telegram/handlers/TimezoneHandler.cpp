#include "TimezoneHandler.h"
#include "../../keyboards/TelegramKeyboards.h"
#include "../../config/Config.h"

// Normalize Unicode punctuation
static String normalizeTZ(String s) {
    s.toLowerCase();
    s.trim();
    s.replace("–", "-");
    s.replace("—", "-");
    s.replace("‑", "-");
    s.replace("＋", "+");
    s.replace("／", "/");
    while (s.indexOf("  ") >= 0) s.replace("  ", " ");
    return s;
}

void TimezoneHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    // ---------------------------------------------------------
    // BACK BUTTON
    // ---------------------------------------------------------
    if (evt.type == EVT_BACK) {
        client->sendMessageWithKeyboard(
            evt.chatId,
            "⚙️ *SETTINGS*\n━━━━━━━━━━━━━━━\nAdjust system configuration below.",
            kbSettings()
        );
        return;
    }

    // ---------------------------------------------------------
    // EUROPE OTHER (placeholder)
    // ---------------------------------------------------------
    if (evt.type == EVT_TIMEZONE_OTHER_EUROPE) {
        client->sendMessageWithKeyboard(
            evt.chatId,
            "🇪🇺 Europe Other is not implemented yet.\n"
            "Try: *UK*, *CET*, *EET* or type a city like *Paris* or *Berlin*.",
            kbTimezone()
        );
        return;
    }

    // ---------------------------------------------------------
    // SET TIMEZONE (typed or button)
    // ---------------------------------------------------------
    if (evt.type == EVT_SET_TIMEZONE) {

        String t = normalizeTZ(evt.text);

        // ---------- US ----------
        if (t == "pacific" || t == "pst" || t == "utc-8" || t == "-8" || t == "pacific (utc-8)") {
            Config_setTimezone("PST8PDT,M3.2.0,M11.1.0");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Pacific (UTC-8)*", kbSettings());
            return;
        }

        if (t == "mountain" || t == "mst" || t == "utc-7" || t == "-7" || t == "mountain (utc-7)") {
            Config_setTimezone("MST7MDT,M3.2.0,M11.1.0");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Mountain (UTC-7)*", kbSettings());
            return;
        }

        if (t == "central" || t == "cst" || t == "utc-6" || t == "-6" || t == "central (utc-6)") {
            Config_setTimezone("CST6CDT,M3.2.0,M11.1.0");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Central (UTC-6)*", kbSettings());
            return;
        }

        if (t == "eastern" || t == "est" || t == "utc-5" || t == "-5" || t == "eastern (utc-5)") {
            Config_setTimezone("EST5EDT,M3.2.0,M11.1.0");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Eastern (UTC-5)*", kbSettings());
            return;
        }

        // ---------- EUROPE ----------
        if (t == "gmt" || t == "uk" || t == "london" || t == "utc" || t == "+0" || t == "uk / gmt (utc+0)") {
            Config_setTimezone("GMT0BST,M3.5.0/1,M10.5.0/2");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *UK / GMT (UTC+0)*", kbSettings());
            return;
        }

        if (t == "cet" || t == "central europe" || t == "paris" || t == "berlin" ||
            t == "utc+1" || t == "+1" || t == "cet (utc+1)") {
            Config_setTimezone("CET-1CEST,M3.5.0/2,M10.5.0/3");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Central Europe (UTC+1)*", kbSettings());
            return;
        }

        if (t == "eet" || t == "eastern europe" || t == "athens" ||
            t == "utc+2" || t == "+2" || t == "eet (utc+2)") {
            Config_setTimezone("EET-2EEST,M3.5.0/3,M10.5.0/4");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Eastern Europe (UTC+2)*", kbSettings());
            return;
        }

        // ---------- AUSTRALIA ----------
        if (t == "aest" || t == "sydney" || t == "melbourne" ||
            t == "utc+10" || t == "+10" || t == "aest (utc+10)") {
            Config_setTimezone("AEST-10AEDT,M10.1.0,M4.1.0/3");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Australia East (UTC+10)*", kbSettings());
            return;
        }

        if (t == "acst" || t == "adelaide" ||
            t == "utc+9:30" || t == "+9:30" || t == "acst (utc+9:30)") {
            Config_setTimezone("ACST-9:30ACDT,M10.1.0,M4.1.0/3");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Australia Central (UTC+9:30)*", kbSettings());
            return;
        }

        if (t == "awst" || t == "perth" ||
            t == "utc+8" || t == "+8" || t == "awst (utc+8)") {
            Config_setTimezone("AWST-8");
            Config_save();
            client->sendMessageWithKeyboard(evt.chatId, "⏱️ Timezone set to *Australia West (UTC+8)*", kbSettings());
            return;
        }

        // Unknown
        client->sendMessageWithKeyboard(
            evt.chatId,
            "❓ I didn’t recognize that timezone.\nTry: *central*, *pst*, *utc+1*, *london*, *sydney*",
            kbSettings()
        );
        return;
    }

    // ---------------------------------------------------------
    // 3. Show timezone menu
    // ---------------------------------------------------------
    client->sendMessageWithKeyboard(
        evt.chatId,
        "🕒 *TIMEZONE SETTINGS*\n━━━━━━━━━━━━━━━\nChoose your local timezone below.",
        kbTimezone()
    );
}