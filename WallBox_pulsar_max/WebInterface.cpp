#include "WebInterface.h"
#include "index_html.h"
#include <ArduinoJson.h>

WebInterface Web;

WebInterface::WebInterface() : _server(80), _ws("/ws") {}

void WebInterface::begin() {
    SystemConfig cfg = Config.get();
    
    // Connect WiFi
    WiFi.mode(WIFI_STA);
    if (cfg.wifi_ssid.length() > 0) {
        WiFi.begin(cfg.wifi_ssid.c_str(), cfg.wifi_pass.c_str());
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            delay(500);
            Serial.print(".");
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
        _apMode = false;
    } else {
        // Fallback AP
        Serial.println("\nWiFi Failed. Starting AP.");
        _apMode = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP("Wallbox-Emulator-AP");
        _dnsServer.start(53, "*", WiFi.softAPIP());
        Serial.println("AP IP: " + WiFi.softAPIP().toString());
    }

    // WebSocket
    _ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
        if(type == WS_EVT_CONNECT){
            client->text("Connected to Log Stream");
        }
    });
    _server.addHandler(&_ws);

    setupRoutes();
    _server.begin();
}

void WebInterface::setupRoutes() {
    // Serve Index
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", INDEX_HTML);
    });

    // Config JSON API (for populating form)
    _server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request){
        SystemConfig cfg = Config.get();
        JsonDocument doc;
        doc["ssid"] = cfg.wifi_ssid;
        doc["pass"] = cfg.wifi_pass;
        doc["m_server"] = cfg.mqtt_server;
        doc["m_port"] = cfg.mqtt_port;
        doc["m_user"] = cfg.mqtt_user;
        doc["m_pass"] = cfg.mqtt_pass;
        doc["m_topic"] = cfg.mqtt_topic;
        doc["s_ip"] = cfg.shelly_ip;
        doc["s_chan"] = cfg.shelly_channel;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Save Config
    _server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
        SystemConfig cfg;
        if(request->hasParam("ssid", true)) cfg.wifi_ssid = request->getParam("ssid", true)->value();
        if(request->hasParam("pass", true)) cfg.wifi_pass = request->getParam("pass", true)->value();
        
        if(request->hasParam("m_server", true)) cfg.mqtt_server = request->getParam("m_server", true)->value();
        if(request->hasParam("m_port", true)) cfg.mqtt_port = request->getParam("m_port", true)->value().toInt();
        if(request->hasParam("m_user", true)) cfg.mqtt_user = request->getParam("m_user", true)->value();
        if(request->hasParam("m_pass", true)) cfg.mqtt_pass = request->getParam("m_pass", true)->value();
        if(request->hasParam("m_topic", true)) cfg.mqtt_topic = request->getParam("m_topic", true)->value();
        
        if(request->hasParam("s_ip", true)) cfg.shelly_ip = request->getParam("s_ip", true)->value();
        if(request->hasParam("s_chan", true)) cfg.shelly_channel = request->getParam("s_chan", true)->value().toInt();
        
        Config.save(cfg);
        request->send(200, "text/plain", "Saved. Rebooting...");
        delay(1000);
        ESP.restart();
    });
    
    // Captive Portal
    if (_apMode) {
        _server.onNotFound([](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", INDEX_HTML);
        });
    }
}

void WebInterface::loop() {
    if (_apMode) {
        _dnsServer.processNextRequest();
    }
    _ws.cleanupClients();
}

void WebInterface::log(String msg) {
    Serial.println(msg);
    _ws.textAll(msg);
}

// Global Helper
void webLog(String msg) {
    Web.log(msg);
}
