#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include "Config.h"

class WebInterface {
public:
    WebInterface();
    void begin();
    void loop();
    void log(String msg);

private:
    AsyncWebServer _server;
    AsyncWebSocket _ws;
    DNSServer _dnsServer;
    
    bool _apMode = false;
    
    void setupRoutes();
    void connectWiFi();
};

extern WebInterface Web;
void webLog(String msg);

#endif
