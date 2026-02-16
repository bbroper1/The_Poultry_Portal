#pragma once
#include <Arduino.h>
#include <time.h>

// ---------------------------------------------------------
// In-memory circular log buffer (system-wide)
// ---------------------------------------------------------

#ifndef MAX_LOGS
#define MAX_LOGS 50
#endif

struct LogEntry {
    time_t timestamp;
    String message;
};

// Add a new log entry
void addLog(const String& msg);

// Number of logs currently stored
int getLogCount();

// Retrieve a log by index (0 = oldest)
LogEntry getLog(int index);

// Clear all logs
void clearLogs();

// NEW: Get the most recent log entry as a string
String Log_getLastAction();

// Optional helper: format a log entry as "YYYY-MM-DD HH:MM:SS - message"
String Log_formatEntry(const LogEntry& e);