#pragma once
#include <Arduino.h>

// Maximum number of log entries stored in memory
#ifndef MAX_LOGS
#define MAX_LOGS 50
#endif

// A single log entry
struct LogEntry {
    time_t timestamp;
    String message;
};

// Add a new log entry
void addLog(const String& msg);

// Access logs
int getLogCount();
LogEntry getLog(int index);

// Clear all logs
void clearLogs();
