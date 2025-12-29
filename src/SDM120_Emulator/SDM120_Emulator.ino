#include <ArduinoOTA.h>

#include "ConfigManager.h"
#include "WiFiManager.h"
#include "HomeAssistantClient.h"
#include "ShellyClient.h"
#include "ShellyGen2Client.h"
#include "WebServerManager.h"
#include "Logger.h"
#include "ModbusManager.h"
#include "DataManager.h"

ConfigManager configManager;
WiFiManager wifiManager;
HomeAssistantClient haClientObj;
ShellyClient shellyClientObj;
ShellyGen2Client shellyGen2ClientObj;
DataManager* pxDataClient = nullptr;
WebServerManager webServer;
Logger& logger = Logger::getInstance(); 
ModbusManager modbusManager;

#define MODBUS_RTU_RX_PIN 18
#define MODBUS_RTU_TX_PIN 19
#define MODBUS_RTU_DE_RE_PIN -1 // Define the DE/RE pin (set to -1 for no control)
#define SDM_UART_BAUD 9600 // SDM120 Modbus baud rate

const int FORCE_AP_BUTTON_PIN = 0; 
const long HAFetchInterval = 500; 
unsigned long lastHAFetchTime = 0;

void setup() {
    Serial.begin(115200);
    logger.log(LOG_TAG_GENERIC, "Starting...");

    configManager.begin();

    pinMode(FORCE_AP_BUTTON_PIN, INPUT_PULLUP);
    delay(100);
    bool forceAP = (digitalRead(FORCE_AP_BUTTON_PIN) == LOW);

    wifiManager.begin(configManager, forceAP);

    // Choose data source based on stored config
    String ds = String(configManager.currentConfig.data_source);
    if (ds == "ShellyGen1") {
        pxDataClient = &shellyClientObj;
        logger.log(LOG_TAG_GENERIC, "Data source: Shelly 3EM (Gen 1)");
    } else if (ds == "ShellyGen2") {
        pxDataClient = &shellyGen2ClientObj;
        logger.log(LOG_TAG_GENERIC, "Data source: Shelly 3EM (Gen 2/3)");
    } else {
        pxDataClient = &haClientObj;
        logger.log(LOG_TAG_GENERIC, "Data source: Home Assistant");
    }

    webServer.begin(configManager, wifiManager, pxDataClient);
    modbusManager.begin(*pxDataClient, 1, MODBUS_RTU_RX_PIN, MODBUS_RTU_TX_PIN, MODBUS_RTU_DE_RE_PIN, SDM_UART_BAUD);

    // --- Inizializzazione ArduinoOTA ---
    ArduinoOTA.setHostname("SDM120_ESP32");
    ArduinoOTA.onStart([]() {
        logger.log(LOG_TAG_GENERIC, "OTA Update Start");
    });
    ArduinoOTA.onEnd([]() {
        logger.log(LOG_TAG_GENERIC, "OTA Update End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        char buf[64];
        snprintf(buf, sizeof(buf), "OTA Progress: %u%%", (progress * 100) / total);
        logger.log(LOG_TAG_GENERIC, buf);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        char buf[64];
        snprintf(buf, sizeof(buf), "OTA Error[%u]", error);
        logger.log(LOG_TAG_GENERIC, buf);
    });
    ArduinoOTA.begin();
    logger.log(LOG_TAG_GENERIC, "OTA Ready");
    logger.log(LOG_TAG_GENERIC, "Setup completed.");
}

void loop() {
    webServer.handleClient();

    if (wifiManager.isConnected() && pxDataClient && (millis() - lastHAFetchTime > HAFetchInterval)) {
        pxDataClient->fetchAllData(configManager);
        lastHAFetchTime = millis();
    }

    modbusManager.modbus_loop();
    ArduinoOTA.handle();
    yield();
}