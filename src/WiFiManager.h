#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "ConfigManager.h"

class WiFiManager {
public:
    void begin(ConfigManager& config, bool forceAP);
    bool isConnected();
    String getStatusString();
    String scanNetworks();

private:
    void startAPMode();
    void connectToWiFi(const char* ssid, const char* password);
    String ap_ssid;
    const char* ap_password = "password123";
};

#endif