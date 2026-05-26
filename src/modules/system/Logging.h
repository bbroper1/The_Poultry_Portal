#pragma once
#include <Arduino.h>
#include <time.h>

// ---------------------------------------------------------
// In-memory circular log buffer (system-wide)
// ---------------------------------------------------------

#ifndef MAX_LOGS
#define MAX_LOGS 50
#endif

enum LogLevel {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

struct LogEntry {
    time_t timestamp;
    String message;
    LogLevel level;
};

// Add a new log entry
void addLog(const String& msg, LogLevel level = LOG_INFO);

// Number of logs currently stored
int getLogCount();

// Retrieve a log by index (0 = oldest)
LogEntry getLog(int index);

// Clear all logs
void clearLogs();

// Get the most recent log entry as a formatted string
String Log_getLastAction();

// Format a log entry as "YYYY-MM-DD HH:MM:SS – message"
String Log_formatEntry(const LogEntry& e);

// Dump all logs into a single string (Telegram-friendly)
String Log_dumpAll();
