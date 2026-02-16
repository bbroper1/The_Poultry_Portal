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
void addLog(const String& msg) {
    time_t now = time(nullptr);

    s_eventLogs[s_logIndex].timestamp = now;
    s_eventLogs[s_logIndex].message   = msg;

    s_logIndex = (s_logIndex + 1) % MAX_LOGS;

    if (s_logCount < MAX_LOGS)
        s_logCount++;
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
    LogEntry empty = {0, ""};

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
    }
}

// ---------------------------------------------------------
// NEW: Return the most recent log entry as a string
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
// Optional helper: format a log entry nicely
// ---------------------------------------------------------
String Log_formatEntry(const LogEntry& e) {
    if (e.timestamp == 0)
        return "none";

    String ts = TimeUtils::formatTimestamp(e.timestamp);
    return ts + " – " + e.message;
}