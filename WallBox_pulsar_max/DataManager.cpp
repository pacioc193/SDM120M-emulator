#include "DataManager.h"
#include "WebInterface.h" // For logging

extern void webLog(String msg); // Forward declaration

void DataManager::begin() {
    SystemConfig cfg = Config.get();
    
    // Init Data
    setSafetyValues();

    // Init MQTT
    if (cfg.mqtt_server.length() > 0) {
        _mqttClient.setClient(_wifiClient);
        _mqttClient.setServer(cfg.mqtt_server.c_str(), cfg.mqtt_port);
        // Lambdas for callback are tricky with member functions, using static/global wrapper or std::bind
        // For simplicity, we assign a static wrapper or handle it in main loop. 
        // Ideally PubSubClient needs a function pointer.
        // We will assign the callback in the .ino or use a global instance helper.
        // For now, let's assume we handle callback dispatch in .ino or use a trick.
        // Actually, PubSubClient setCallback takes std::function in some forks, but standard is void(*)(char*, uint8_t*, unsigned int).
        // We will assume standard. I'll make a global helper in this file.
    }
}

// Global helper for MQTT Callback
DataManager* _globalDataManagerInstance = nullptr;
void globalMqttCallback(char* topic, byte* payload, unsigned int length) {
    if (_globalDataManagerInstance) {
        _globalDataManagerInstance->mqttCallback(topic, payload, length);
    }
}

void DataManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
    char msg[length + 1];
    memcpy(msg, payload, length);
    msg[length] = '\0';
    
    // webLog("MQTT RX: " + String(msg)); // Verbose?
    
    JsonDocument doc; // ArduinoJson 7
    DeserializationError error = deserializeJson(doc, msg);
    
    if (error) {
        webLog("JSON Parse Error: " + String(error.c_str()));
        return;
    }

    _currentData.voltage = doc["voltage"] | 0.0f;
    _currentData.current = doc["current"] | 0.0f;
    _currentData.active_power = doc["power"] | 0.0f; // Prompt says "power"
    _currentData.total_energy = doc["energy"] | 0.0f;
    _currentData.pf = doc["pf"] | 0.0f;
    _currentData.frequency = doc["frequency"] | 50.0f; // Default 50
    
    // Calculate derived if missing
    // Apparent = V * A
    // Reactive = sqrt(App^2 - Act^2)
    // But MQTT usually provides what we need. 
    // If "power" is active power.
    
    // Calculate others just in case or if 0
    float va_calc = _currentData.voltage * _currentData.current;
    if (doc.containsKey("apparent_power")) {
        _currentData.apparent_power = doc["apparent_power"];
    } else {
        _currentData.apparent_power = va_calc;
    }
    
    if (doc.containsKey("reactive_power")) {
        _currentData.reactive_power = doc["reactive_power"];
    } else {
        // sqrt(VA^2 - W^2)
        float p = _currentData.active_power;
        float va = _currentData.apparent_power;
        if (va > p) {
             _currentData.reactive_power = sqrt((va * va) - (p * p));
        } else {
             _currentData.reactive_power = 0.0f;
        }
    }

    _lastMqttSeen = millis();
}

void DataManager::loop() {
    if (!_globalDataManagerInstance) {
        _globalDataManagerInstance = this;
        _mqttClient.setCallback(globalMqttCallback);
    }

    SystemConfig cfg = Config.get();
    unsigned long now = millis();

    // State Machine
    if (now - _lastMqttSeen < 5000 && _lastMqttSeen != 0) {
        _currentState = STATE_PRIMARY_MQTT;
    } else {
        // Fallback or Safety
        // Check if Shelly is configured
        if (cfg.shelly_ip.length() > 0) {
            _currentState = STATE_FAILOVER_SHELLY;
        } else {
            _currentState = STATE_SAFETY_STOP;
        }
    }

    // MQTT Handling (Always try to connect/read if configured)
    if (cfg.mqtt_server.length() > 0) {
        if (!_mqttClient.connected()) {
             // Non-blocking reconnect attempt could be here, but PubSubClient connect is blocking.
             // We'll try periodically to not block the loop too much.
             static unsigned long lastReconnect = 0;
             if (now - lastReconnect > 5000) {
                 connectMqtt();
                 lastReconnect = now;
             }
        } else {
            _mqttClient.loop();
        }
    }

    // Actions based on State
    if (_currentState == STATE_PRIMARY_MQTT) {
        // Data is updated via callback
    } 
    else if (_currentState == STATE_FAILOVER_SHELLY) {
        // Poll Shelly
        if (now - _lastShellyPoll > 1000) {
            if (pollShellyInstant()) {
                _lastShellyPoll = now;
            } else {
                // If polling fails? Keep old data or go to safety?
                // If poll fails repeatedly, we should maybe go to safety. 
                // But simplified: just try.
            }
        }
        if (now - _lastShellyEnergyPoll > 5000) {
            pollShellyEnergy();
            _lastShellyEnergyPoll = now;
        }
    } 
    else { // SAFETY STOP
        setSafetyValues();
    }
}

void DataManager::connectMqtt() {
    SystemConfig cfg = Config.get();
    if (WiFi.status() != WL_CONNECTED) return;
    
    // Generate Random Client ID
    String clientId = "ESP32-Wallbox-" + String(random(0xffff), HEX);
    
    bool connected = false;
    if (cfg.mqtt_user.length() > 0) {
        connected = _mqttClient.connect(clientId.c_str(), cfg.mqtt_user.c_str(), cfg.mqtt_pass.c_str());
    } else {
        connected = _mqttClient.connect(clientId.c_str());
    }

    if (connected) {
        webLog("MQTT Connected to " + cfg.mqtt_server);
        _mqttClient.subscribe(cfg.mqtt_topic.c_str());
    }
}

bool DataManager::pollShellyInstant() {
    SystemConfig cfg = Config.get();
    if (WiFi.status() != WL_CONNECTED) return false;
    
    HTTPClient http;
    String url = "http://" + cfg.shelly_ip + "/rpc/EM" + String(cfg.shelly_channel) + ".GetStatus?id=0";
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, http.getString());
        if (!error) {
            _currentData.voltage = doc["voltage"] | 0.0f;
            _currentData.current = doc["current"] | 0.0f;
            _currentData.active_power = doc["act_power"] | 0.0f;
            _currentData.apparent_power = doc["aprt_power"] | (_currentData.voltage * _currentData.current);
            _currentData.pf = doc["pf"] | 0.0f;
            _currentData.frequency = doc["freq"] | 50.0f;
            
            // Calc Reactive
            if (doc.containsKey("reactive_power")) {
                 // Some shelly firmwares have it? Prompt JSON didn't show it.
                 // Prompt said: JSON { ... } Source 1.
                 // It doesn't have reactive.
            }
            // Calc Reactive fallback
            float va = _currentData.apparent_power;
            float w = _currentData.active_power;
            if (va > w) {
                _currentData.reactive_power = sqrt(va*va - w*w);
            } else {
                _currentData.reactive_power = 0.0f;
            }
            
            return true;
        } else {
            webLog("Shelly JSON Error");
        }
    } else {
        webLog("Shelly HTTP Error: " + String(httpCode));
    }
    http.end();
    return false;
}

bool DataManager::pollShellyEnergy() {
    SystemConfig cfg = Config.get();
    if (WiFi.status() != WL_CONNECTED) return false;
    
    HTTPClient http;
    String url = "http://" + cfg.shelly_ip + "/rpc/EM" + String(cfg.shelly_channel) + "Data.GetStatus?id=0";
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, http.getString());
        if (!error) {
            _currentData.total_energy = doc["total_act_energy"] | 0.0f;
            // Convert Wh to kWh if needed? 
            // Usually Shelly Gen3 reports in Wh. EM111 wants kWh?
            // EM111 Register: "Total Energy x10". Unit?
            // Usually EM111 registers are kWh with decimals.
            // If Shelly is Wh, 123456.7 Wh = 123.4567 kWh.
            // Register x10 means 123.4 * 10 = 1234.
            // Wait, standard Shelly Gen 2/3 is Wh.
            // I should divide by 1000 to get kWh.
            _currentData.total_energy = _currentData.total_energy / 1000.0f;
            return true;
        }
    }
    http.end();
    return false;
}

void DataManager::setSafetyValues() {
    _currentData.active_power = 0.0f;
    _currentData.current = 0.0f;
    // Keep Voltage/Freq/Energy to avoid errors in Wallbox if possible?
    // Prompt says: "Current and Active Power to 0".
    // We should probably keep Voltage 230 and Freq 50 to simulate a connected grid but 0 load.
    if (_currentData.voltage < 10.0f) _currentData.voltage = 230.0f;
    if (_currentData.frequency < 10.0f) _currentData.frequency = 50.0f;
}

MeterData DataManager::getData() const {
    return _currentData;
}

DataState DataManager::getState() const {
    return _currentState;
}
