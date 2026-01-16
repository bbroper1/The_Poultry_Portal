#include "Logging.h"
#include <time.h>

// Internal circular buffer
static LogEntry eventLogs[MAX_LOGS];
static int logIndex = 0;
static int logCount = 0;

// Add a new log entry
void addLog(const String& msg) {
    time_t now = time(nullptr);

    eventLogs[logIndex].timestamp = now;
    eventLogs[logIndex].message   = msg;

    logIndex = (logIndex + 1) % MAX_LOGS;

    if (logCount < MAX_LOGS)
        logCount++;
}

// Number of logs currently stored
int getLogCount() {
    return logCount;
}

// Retrieve a log by index (0 = oldest)
LogEntry getLog(int index) {
    LogEntry empty = {0, ""};

    if (index < 0 || index >= logCount)
        return empty;

    int realIndex = (logIndex - logCount + index + MAX_LOGS) % MAX_LOGS;
    return eventLogs[realIndex];
}

// Clear all logs
void clearLogs() {
    logIndex = 0;
    logCount = 0;

    for (int i = 0; i < MAX_LOGS; i++) {
        eventLogs[i].timestamp = 0;
        eventLogs[i].message   = "";
    }
}
