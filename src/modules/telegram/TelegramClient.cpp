#include "TelegramClient.h"
#include <ArduinoJson.h>

TelegramClient* TelegramClient::instance = nullptr;

// ---------------------------------------------------------
// Constructor
// ---------------------------------------------------------
TelegramClient::TelegramClient(const String& token, const char* rootCert)
    : token(token)
{
    instance = this;  // ⭐ Global pointer for SupervisorTask

    client.setCACert(rootCert);
    client.setTimeout(100);   // Fast read timeout
}

// ---------------------------------------------------------
// JSON escape helper
// ---------------------------------------------------------
String TelegramClient::escapeJSON(const String& s) {
    String out;
    out.reserve(s.length());

    for (char c : s) {
        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c; break;
        }
    }
    return out;
}

// ---------------------------------------------------------
// Build full API URL
// ---------------------------------------------------------
String TelegramClient::buildURL(const String& method) {
    return "https://api.telegram.org/bot" + token + "/" + method;
}

// ---------------------------------------------------------
// Efficient HTTPS GET
// ---------------------------------------------------------
bool TelegramClient::httpsGET(const String& url, String& response) {

    if (!client.connect("api.telegram.org", 443)) {
        Serial.println("[TG] HTTPS GET connect failed");
        return false;
    }

    client.print(
        String("GET ") + url + " HTTP/1.1\r\n" +
        "Host: api.telegram.org\r\n" +
        "Connection: close\r\n\r\n"
    );

    response = "";

    while (client.connected() || client.available()) {
        while (client.available()) {
            response += (char)client.read();
        }
        delay(2);  // ⭐ Prevent WiFi starvation
    }

    return true;
}

// ---------------------------------------------------------
// Efficient HTTPS POST
// ---------------------------------------------------------
bool TelegramClient::httpsPOST(const String& url, const String& body, String& response) {

    if (!client.connect("api.telegram.org", 443)) {
        Serial.println("[TG] HTTPS POST connect failed");
        return false;
    }

    client.print(
        String("POST ") + url + " HTTP/1.1\r\n" +
        "Host: api.telegram.org\r\n" +
        "Content-Type: application/json\r\n" +
        "Content-Length: " + body.length() + "\r\n" +
        "Connection: close\r\n\r\n" +
        body
    );

    response = "";

    while (client.connected() || client.available()) {
        while (client.available()) {
            response += (char)client.read();
        }
        delay(2);  // ⭐ Prevent WiFi starvation
    }

    return true;
}

// ---------------------------------------------------------
// Send plain text message
// ---------------------------------------------------------
bool TelegramClient::sendMessage(uint64_t chatId, const String& text) {

    String url = buildURL("sendMessage");

    String payload =
        "{\"chat_id\":" + String(chatId) +
        ",\"text\":\"" + escapeJSON(text) + "\"}";

    String response;
    return httpsPOST(url, payload, response);
}

// ---------------------------------------------------------
// Send message with keyboard
// ---------------------------------------------------------
bool TelegramClient::sendMessageWithKeyboard(
    uint64_t chatId,
    const String& text,
    const String& keyboardJson
) {
    String url = buildURL("sendMessage");

    String payload =
        "{\"chat_id\":" + String(chatId) +
        ",\"text\":\"" + escapeJSON(text) +
        "\",\"reply_markup\":" + keyboardJson + "}";

    String response;
    return httpsPOST(url, payload, response);
}

// ---------------------------------------------------------
// Send photo by URL
// ---------------------------------------------------------
bool TelegramClient::sendPhotoByUrl(uint64_t chatId, const String& url, const String& caption) {

    String apiUrl = buildURL("sendPhoto");

    String payload = "{";
    payload += "\"chat_id\":" + String(chatId) + ",";
    payload += "\"photo\":\"" + escapeJSON(url) + "\"";

    if (caption.length() > 0) {
        payload += ",\"caption\":\"" + escapeJSON(caption) + "\"";
    }

    payload += "}";

    String response;
    return httpsPOST(apiUrl, payload, response);
}

// ---------------------------------------------------------
// Raw getUpdates() (JSON string)
// ---------------------------------------------------------
String TelegramClient::getUpdates() {

    String url = buildURL(
        "getUpdates?timeout=1&offset=" + String(lastUpdateId + 1)
    );

    String response;
    httpsGET(url, response);
    return response;
}

// ---------------------------------------------------------
// Polling API to get next update
// ---------------------------------------------------------
bool TelegramClient::getNextUpdate(TelegramUpdate& out) {

    String json = getUpdates();

    int start = json.indexOf('{');
    int end   = json.lastIndexOf('}');
    if (start < 0 || end <= start) {
        Serial.println("[TG] Invalid JSON bounds");
        return false;
    }

    json = json.substring(start, end + 1);

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Serial.print("[TG] JSON parse error: ");
        Serial.println(err.c_str());
        return false;
    }

    JsonArray results = doc["result"].as<JsonArray>();
    if (results.size() == 0) {
        return false;
    }

    JsonObject update = results[0];

    lastUpdateId = update["update_id"] | lastUpdateId;

    // CALLBACK QUERY
    if (update.containsKey("callback_query")) {
        JsonObject cb = update["callback_query"];

        out.chatId = cb["message"]["chat"]["id"].as<String>();
        out.text   = cb["data"].as<String>();

        return true;
    }

    // NORMAL MESSAGE
    if (update.containsKey("message")) {
        JsonObject msg = update["message"];

        out.chatId = msg["chat"]["id"].as<String>();

        if (msg.containsKey("text"))
            out.text = msg["text"].as<String>();
        else
            out.text = "";

        if (msg.containsKey("location")) {
            JsonObject loc = msg["location"];
            out.latitude  = loc["latitude"]  | 0.0f;
            out.longitude = loc["longitude"] | 0.0f;
        }

        return true;
    }

    return false;
}
