#include "Logging.h"
#include "modules/utils/TimeUtils.h"

// ---------------------------------------------------------
// Internal circular buffer
// ---------------------------------------------------------
static LogEntry s_eventLogs[MAX_LOGS];
static int s_logIndex = 0;   // next write position
static int s_logCount = 0;   // number of valid entries

// ---------------------------------------------------------
// Add a new log entry
// ---------------------------------------------------------
void addLog(const String& msg, LogLevel level) {
    time_t now = time(nullptr);

    s_eventLogs[s_logIndex].timestamp = now;
    s_eventLogs[s_logIndex].message   = msg;
    s_eventLogs[s_logIndex].level     = level;

    s_logIndex = (s_logIndex + 1) % MAX_LOGS;

    if (s_logCount < MAX_LOGS)
        s_logCount++;

    // Optional: also print to Serial
    // Serial.println(Log_formatEntry(s_eventLogs[(s_logIndex - 1 + MAX_LOGS) % MAX_LOGS]));
}

// ---------------------------------------------------------
// Number of logs currently stored
// ---------------------------------------------------------
int getLogCount() {
    return s_logCount;
}

// ---------------------------------------------------------
// Retrieve a log by index (0 = oldest)
// ---------------------------------------------------------
LogEntry getLog(int index) {
    LogEntry empty = {0, "", LOG_INFO};

    if (index < 0 || index >= s_logCount)
        return empty;

    int realIndex = (s_logIndex - s_logCount + index + MAX_LOGS) % MAX_LOGS;
    return s_eventLogs[realIndex];
}

// ---------------------------------------------------------
// Clear all logs
// ---------------------------------------------------------
void clearLogs() {
    s_logIndex = 0;
    s_logCount = 0;

    for (int i = 0; i < MAX_LOGS; i++) {
        s_eventLogs[i].timestamp = 0;
        s_eventLogs[i].message   = "";
        s_eventLogs[i].level     = LOG_INFO;
    }
}

// ---------------------------------------------------------
// Return the most recent log entry as a string
// ---------------------------------------------------------
String Log_getLastAction() {
    if (s_logCount == 0)
        return "none";

    int idx = s_logIndex - 1;
    if (idx < 0)
        idx = MAX_LOGS - 1;

    const LogEntry& e = s_eventLogs[idx];

    if (e.timestamp == 0)
        return "none";

    return Log_formatEntry(e);
}

// ---------------------------------------------------------
// Format a log entry nicely
// ---------------------------------------------------------
String Log_formatEntry(const LogEntry& e) {
    if (e.timestamp == 0)
        return "none";

    String ts = TimeUtils::formatTimestamp(e.timestamp);

    String lvl;
    switch (e.level) {
        case LOG_INFO:  lvl = "[INFO] "; break;
        case LOG_WARN:  lvl = "[WARN] "; break;
        case LOG_ERROR: lvl = "[ERR]  "; break;
    }

    return ts + " " + lvl + e.message;
}

// ---------------------------------------------------------
// Dump all logs into a single string (Telegram-friendly)
// ---------------------------------------------------------
String Log_dumpAll() {
    String out;
    out.reserve(2000);

    for (int i = 0; i < s_logCount; i++) {
        LogEntry e = getLog(i);
        out += Log_formatEntry(e) + "\n";
    }

    return out;
}
