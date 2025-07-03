#include "Logger.h"

Logger::Logger() {
    logEntries.reserve(MAX_LOG_ENTRIES);
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// Add a new log, removing the oldest if necessary
void Logger::log(LogTag tag, const String& message) {
    if (logEntries.size() >= MAX_LOG_ENTRIES) {
        logEntries.erase(logEntries.begin());
    }

    LogEntry entry;
    entry.timestamp = String(millis() / 1000);
    entry.tag = tagToString(tag);
    entry.message = message;
    logEntries.push_back(entry);

    if (tag != LOG_TAG_HA) { // Don't print Home Assistant logs to serial
        Serial.println("[" + entry.tag + "] " + message);
    }
}

String Logger::getLogsHtml() {
    String html = "";
    for (int i = logEntries.size() - 1; i >= 0; i--) { // Show the most recent first
        html += "<tr>";
        html += "<td>" + logEntries[i].timestamp + "s</td>";
        html += "<td>" + logEntries[i].tag + "</td>";
        html += "<td>" + logEntries[i].message + "</td>";
        html += "</tr>";
    }
    return html;
}

// Format logs as plain text
String Logger::getLogsTxt() {
    String txt = "Timestamp (s),Tag,Message\r\n";
    for (const auto& entry : logEntries) {
        txt += entry.timestamp + "," + entry.tag + "," + entry.message + "\r\n";
    }
    return txt;
}

String Logger::tagToString(LogTag tag) {
    switch (tag) {
    case LOG_TAG_GENERIC:   return "Generic";
    case LOG_TAG_SERVERWEB: return "WebServer";
    case LOG_TAG_WIFI:      return "WiFi";
    case LOG_TAG_HA:        return "HomeAssistant";
    case LOG_TAG_CONFIG:    return "Config";
    case LOG_TAG_MODBUS:    return "Modbus";
    default:                return "Other";
    }
}

String Logger::getLogsHtml(const String& filterTag) {
    String html = "";
    for (int i = logEntries.size() - 1; i >= 0; i--) {
        // Applica il filtro: se il filtro è "All" o se il tag corrisponde, mostra il log
        if (filterTag == "All" || logEntries[i].tag == filterTag) {
            html += "<tr>";
            html += "<td>" + logEntries[i].timestamp + "s</td>";
            html += "<td>" + logEntries[i].tag + "</td>";
            html += "<td>" + logEntries[i].message + "</td>";
            html += "</tr>";
        }
    }
    return html;
}