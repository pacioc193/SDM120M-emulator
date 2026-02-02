#include <WiFi.h>
#include <ArduinoOTA.h>
#include "Config.h"
#include "WebInterface.h"
#include "DataManager.h"
#include "ModbusSlave.h"

DataManager DataMgr;
ModbusSlaveManager ModbusMgr;

void setup() {
    Serial.begin(115200);
    
    // 1. Load Config
    Config.begin();
    
    // 2. Start Web/WiFi
    Web.begin();
    
    // 3. Start Data Manager
    DataMgr.begin();
    
    // 4. Start Modbus
    ModbusMgr.begin();
    
    // 5. Setup OTA
    ArduinoOTA.setHostname("Wallbox-Emulator");
    ArduinoOTA.onStart([]() { webLog("OTA Start"); });
    ArduinoOTA.onEnd([]() { webLog("OTA End"); });
    ArduinoOTA.onError([](ota_error_t error) { webLog("OTA Error"); });
    ArduinoOTA.begin();
    
    webLog("System Started");
}

void loop() {
    // Core Tasks
    Web.loop();
    DataMgr.loop();
    ModbusMgr.loop();
    ArduinoOTA.handle();
    
    // Sync Data: DataManager -> ModbusSlave
    // We update Modbus registers with latest data
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 500) { // Update Modbus regs every 500ms
        MeterData data = DataMgr.getData();
        ModbusMgr.updateData(data);
        
        // Debug Log (Optional, maybe once every 10s)
        static unsigned long lastLog = 0;
        if (millis() - lastLog > 10000) {
            String stateStr;
            switch(DataMgr.getState()) {
                case STATE_PRIMARY_MQTT: stateStr = "MQTT"; break;
                case STATE_FAILOVER_SHELLY: stateStr = "SHELLY"; break;
                case STATE_SAFETY_STOP: stateStr = "SAFETY"; break;
            }
            webLog("State: " + stateStr + " | P: " + String(data.active_power) + "W | E: " + String(data.total_energy) + "kWh");
            lastLog = millis();
        }
        
        lastUpdate = millis();
    }
}
