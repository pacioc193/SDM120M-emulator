#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "HomeAssistantClient.h"
#include "Logger.h"
#include "WiFi.h"

void HomeAssistantClient::fetchAllData(ConfigManager& config) {
    bOnline = false;

    if (WiFi.status() != WL_CONNECTED) {
        Logger::getInstance().log(LOG_TAG_HA, "WiFi not connected, cannot fetch data from Home Assistant.");
        return;
    }

    const String& url = config.currentConfig.home_assistant_url;
    const String& token = config.currentConfig.home_assistant_token;

    if (url.isEmpty() || token.isEmpty()) {
        Logger::getInstance().log(LOG_TAG_HA, "URL or Token for Home Assistant not found...");
        return;
    }

    // Array di entity_id e puntatori ai membri di currentData
    struct EntityInfo {
        const String& entity_id;
        float* value;
        const char* name;
    } entities[] = {
        { config.currentConfig.home_assistant_entity_power,         &currentData.power_w,      "Power" },
        { config.currentConfig.home_assistant_entity_energy,        &currentData.energy_kwh,   "Energy" },
        { config.currentConfig.home_assistant_entity_voltage,       &currentData.voltage_v,    "Voltage" },
        { config.currentConfig.home_assistant_entity_current,       &currentData.current_a,    "Current" },
        { config.currentConfig.home_assistant_entity_frequency,     &currentData.frequency_hz, "Frequency" },
        { config.currentConfig.home_assistant_entity_power_factor,  &currentData.power_factor, "Power Factor" }
    };

    // Fetch e validazione base
    for (size_t i = 0; i < sizeof(entities)/sizeof(entities[0]); ++i) {
        *(entities[i].value) = getHAEntityState(url, token, entities[i].entity_id);
    }

    // Correzione power factor se necessario
    if (currentData.power_factor > 1.0f) {
        currentData.power_factor /= 100.0f;
    }

    // Validazione valori
    struct Validation {
        float value;
        bool valid;
        const char* name;
        String msg;
    } validations[] = {
        { currentData.power_w,      currentData.power_w > 0.0f,      "Power",        "Power is zero or negative." },
        { currentData.energy_kwh,   currentData.energy_kwh >= 0.0f,  "Energy",       "Energy is negative." },
        { currentData.voltage_v,    currentData.voltage_v > 0.0f,    "Voltage",      "Voltage is zero or negative." },
        { currentData.current_a,    currentData.current_a > 0.0f,    "Current",      "Current is zero or negative." },
        { currentData.frequency_hz, currentData.frequency_hz > 0.0f, "Frequency",    "Frequency is zero or negative." },
        { currentData.power_factor, currentData.power_factor >= 0.0f && currentData.power_factor <= 1.0f, "Power Factor", "Power factor is out of range: " + String(currentData.power_factor) }
    };

    for (size_t i = 0; i < sizeof(validations)/sizeof(validations[0]); ++i) {
        if (!validations[i].valid) {
            Logger::getInstance().log(LOG_TAG_HA, validations[i].msg);
            return;
        }
    }

    bOnline = true;
}

float HomeAssistantClient::getHAEntityState(const String& base_url, const String& token, const String& entity_id) {
    if (entity_id.isEmpty()) return 0.0;

    HTTPClient http;
    String url = base_url + "/api/states/" + entity_id;

    http.begin(url);
    http.setTimeout(1500);
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK) {
        String payload = http.getString();
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Logger::getInstance().log(LOG_TAG_HA, "Failed parsing JSON of " + entity_id + ": " + error.f_str());
            http.end();
            return 0.0;
        }

        const char* state_str = doc["state"];
        if (state_str) {
            http.end();
            return String(state_str).toFloat();
        }
        else {
            Logger::getInstance().log(LOG_TAG_HA, "State not found in JSON for " + entity_id);
        }
    }
    else {
        Logger::getInstance().log(LOG_TAG_HA, "HTTP Error " + String(httpResponseCode) + " for " + entity_id);
    }

    http.end();
    return 0.0;
}

String HomeAssistantClient::getDataJson() {
    String json = "{";
    json += "\"Power (Kw)\":" + String(currentData.power_w, 1);
    json += ",\"Energy (Kwh)\":" + String(currentData.energy_kwh, 0);
    json += ",\"Voltage (V)\":" + String(currentData.voltage_v, 1);
    json += ",\"Current (A)\":" + String(currentData.current_a, 1);
    json += ",\"Frequency (Hz)\":" + String(currentData.frequency_hz, 1);
    json += ",\"Power Factor\":" + String(currentData.power_factor, 2);
    json += "}";
    return json;
}