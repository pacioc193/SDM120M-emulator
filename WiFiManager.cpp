#include "WiFiManager.h"
#include "Logger.h"

void WiFiManager::begin(ConfigManager& config, bool forceAP) {
    uint64_t chipid = ESP.getEfuseMac();
    char chipid_str[13];
    sprintf(chipid_str, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
    ap_ssid = "ESP32_SDM120_" + String(chipid_str).substring(8);

    if (forceAP) {
        Logger::getInstance().log(LOG_TAG_WIFI, "Forcing AP mode via button.");
        startAPMode();
    }
    else if (config.currentConfig.configured && strlen(config.currentConfig.ssid) > 0) {
        connectToWiFi(config.currentConfig.ssid, config.currentConfig.password);
    }
    else {
        Logger::getInstance().log(LOG_TAG_WIFI, "No configuration found. Starting in AP mode.");
        startAPMode();
    }
}

void WiFiManager::connectToWiFi(const char* ssid, const char* password) {
    Logger::getInstance().log(LOG_TAG_WIFI, "Attempting to connect to: " + String(ssid));
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    uint64_t chipid = ESP.getEfuseMac();
    char chipid_str[13];
    sprintf(chipid_str, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
	ap_ssid = "SDM120-ESP32_" + String(chipid_str).substring(8);
    WiFi.setHostname(ap_ssid.c_str());

    int maxAttempts = 20;
    while (WiFi.status() != WL_CONNECTED && maxAttempts > 0) {
        delay(500);
        Serial.print(".");
        maxAttempts--;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Logger::getInstance().log(LOG_TAG_WIFI, "Connected! IP address: " + WiFi.localIP().toString());
    }
    else {
        Logger::getInstance().log(LOG_TAG_WIFI, "Connection failed. Starting in AP mode.");
        startAPMode();
    }
}

void WiFiManager::startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid.c_str(), ap_password);
    Logger::getInstance().log(LOG_TAG_WIFI, "Access Point '" + ap_ssid + "' created. IP: " + WiFi.softAPIP().toString());
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getStatusString() {
    if (isConnected()) {
        return "Connected to " + String(WiFi.SSID()) + " (" + WiFi.localIP().toString() + ")";
    }
    return "Not Connected. In AP mode (" + ap_ssid + ")";
}

String WiFiManager::scanNetworks() {
    Logger::getInstance().log(LOG_TAG_WIFI, "Starting WiFi network scan...");
    int n = WiFi.scanNetworks();
    String json = "[";
    if (n > 0) {
        for (int i = 0; i < n; ++i) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
        }
    }
    json += "]";
    WiFi.scanDelete();
    Logger::getInstance().log(LOG_TAG_WIFI, "Scan completed. Found " + String(n) + " networks.");
    return json;
}