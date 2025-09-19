#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <vector>

#define MAX_LOG_ENTRIES 200 // Limit the number of logs in memory

enum LogTag {
    LOG_TAG_GENERIC,
    LOG_TAG_SERVERWEB,
    LOG_TAG_WIFI,
    LOG_TAG_HA,
    LOG_TAG_SHELLY, // Added Shelly tag
    LOG_TAG_CONFIG,
    LOG_TAG_MODBUS
};

struct LogEntry {
    String timestamp;
    String tag;
    String message;
};

class Logger {
public:
    static Logger& getInstance();
    void log(LogTag tag, const String& message);
    String getLogsHtml();
    String getLogsTxt();
    String getLogsHtml(const String& filterTag);

private:
    Logger();
    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;

    std::vector<LogEntry> logEntries;
    String tagToString(LogTag tag);
};

#endif