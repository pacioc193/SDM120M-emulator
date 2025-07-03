#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#pragma once

#include <WebServer.h>
#include "ConfigManager.h"
#include "WiFiManager.h"
#include "HomeAssistantClient.h"

class WebServerManager {
public:
    void begin(ConfigManager& config, WiFiManager& wifi, HomeAssistantClient& ha);
    void handleClient();

private:
    WebServer server;
    ConfigManager* pConfig = nullptr;
    WiFiManager* pWifi = nullptr;
    HomeAssistantClient* pHa = nullptr;
    String lastScanJson = "[]";

    void setupRoutes();
    String generateIndexHtml();

    // Handlers
    void handleRoot();
    void handleData();
    void handleWifiStatus();
    void handleScan();
    void handleSaveWifi();
    void handleSaveHomeAssistant();
    void handleLogHtml();
    void handleLogTxt();
    void handleNotFound();
};

#endif