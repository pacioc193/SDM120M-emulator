#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#pragma once

#include <WebServer.h>
#include "ConfigManager.h"
#include "WiFiManager.h"
#include "DataManager.h"

class WebServerManager {
public:
    void begin(ConfigManager& config, WiFiManager& wifi, DataManager* ha);
    void handleClient();

private:
    WebServer server;
    ConfigManager* pConfig = nullptr;
    WiFiManager* pWifi = nullptr;
    DataManager* pClient = nullptr;
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
    void handleRestart();
};

#endif