#include "EnergyHandler.h"
#include "../../keyboards/TelegramKeyboards.h"

// System modules
#include "EnergyModule.h"
#include "TimeUtils.h"

// ---------------------------------------------------------
// Formatting Helpers
// ---------------------------------------------------------

String EnergyHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

String EnergyHandler::kv(const String& label, const String& value) {
    return "• " + label + ": " + value + "\n";
}

// ---------------------------------------------------------
// Sparkline Helper (CLASS METHOD — FIXED)
// ---------------------------------------------------------
String EnergyHandler::makeSparkline(float* data, int len) {
    if (!data || len <= 0) return "";

    float minV = data[0];
    float maxV = data[0];

    for (int i = 1; i < len; i++) {
        if (data[i] < minV) minV = data[i];
        if (data[i] > maxV) maxV = data[i];
    }

    const char* bars[8] = { "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█" };

    String out;
    out.reserve(len * 3);

    if (maxV == minV) {
        for (int i = 0; i < len; i++) out += bars[3];
        return out;
    }

    for (int i = 0; i < len; i++) {
        float norm = (data[i] - minV) / (maxV - minV);
        int idx = (int)(norm * 7.0f + 0.5f);
        if (idx < 0) idx = 0;
        if (idx > 7) idx = 7;
        out += bars[idx];
    }

    return out;
}

// ---------------------------------------------------------
// Handler
// ---------------------------------------------------------
void EnergyHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    String out;
    out.reserve(900);

    // ---------------------------------------------------------
    // SYSTEM ENERGY
    // ---------------------------------------------------------
    out += blockHeader("⚡", "SYSTEM ENERGY");
    out += kv("Today", String(EnergySys_getTodaymAh(), 2) + " mAh");
    out += kv("Avg Daily (30d)", String(EnergySys_getAvgDailymAh(), 2) + " mAh/day");
    out += kv("30-Day Total", String(EnergySys_getMonthlymAh(), 2) + " mAh");
    out += kv("Peak Current", String(EnergySys_getPeakCurrentmA()) + " mA");
    out += kv("Avg Current", String(EnergySys_getAvgCurrentmA()) + " mA");
    out += kv("Last Reset", EnergySys_getLastResetStr());
    out += "\n";

    // ---------------------------------------------------------
    // MOTOR ENERGY
    // ---------------------------------------------------------
    out += blockHeader("🔌", "MOTOR ENERGY");
    out += kv("Today", String(EnergyMotor_getTodaymAh(), 2) + " mAh");
    out += kv("Avg Daily (30d)", String(EnergyMotor_getAvgDailymAh(), 2) + " mAh/day");
    out += kv("30-Day Total", String(EnergyMotor_getMonthlymAh(), 2) + " mAh");
    out += kv("Peak Motor Current", String(EnergyMotor_getPeakCurrentmA()) + " mA");
    out += kv("Avg Motor Current", String(EnergyMotor_getAvgCurrentmA()) + " mA");
    out += kv("Last Reset", EnergyMotor_getLastResetStr());
    out += "\n";

    // ---------------------------------------------------------
    // HISTORY SPARKLINES
    // ---------------------------------------------------------
    out += blockHeader("📈", "HISTORY (30d)");

    float* sysHist = EnergySys_getHistoryArray();
    float* motHist = EnergyMotor_getHistoryArray();

    out += kv("System", makeSparkline(sysHist, 30));
    out += kv("Motor",  makeSparkline(motHist, 30));

    // ---------------------------------------------------------
    // SEND
    // ---------------------------------------------------------
    client->sendMessage(evt.chatId, out);
}
