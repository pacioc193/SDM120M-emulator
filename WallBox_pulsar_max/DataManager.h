#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "ModbusSlave.h"

enum DataState {
    STATE_PRIMARY_MQTT,
    STATE_FAILOVER_SHELLY,
    STATE_SAFETY_STOP
};

class DataManager {
public:
    void begin();
    void loop();
    MeterData getData() const;
    DataState getState() const;
    
    // MQTT Callback must be public for global wrapper access
    void mqttCallback(char* topic, byte* payload, unsigned int length);

private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    
    DataState _currentState = STATE_SAFETY_STOP;
    MeterData _currentData;
    
    unsigned long _lastMqttSeen = 0;
    unsigned long _lastShellyPoll = 0;
    unsigned long _lastShellyEnergyPoll = 0;
    
    // MQTT Handling
    void connectMqtt();
    
    // Shelly Handling
    bool pollShellyInstant();
    bool pollShellyEnergy();
    
    // Safety
    void setSafetyValues();
    
    // Helpers
    void updateState();
};

#endif
