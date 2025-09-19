#include "ShellyClient.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Logger.h"
#include "WiFi.h"

void ShellyClient::fetchAllData(ConfigManager& config) {
    bOnline = false;

    if (WiFi.status() != WL_CONNECTED) {
        Logger::getInstance().log(LOG_TAG_SHELLY, "WiFi not connected, cannot fetch data from Shelly.");
        return;
    }

    // Prefer explicit shelly_url config, fall back to home_assistant_url for compatibility
    String url = String(config.currentConfig.shelly_url);
    if (url.isEmpty()) {
        url = String(config.currentConfig.home_assistant_url);
    }

    if (url.isEmpty()) {
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly URL not found...");
        return;
    }

    // Shelly 3EM status endpoint typically at /status
    String api = url + "/status";

    HTTPClient http;
    http.begin(api);
    http.setTimeout(2000);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Logger::getInstance().log(LOG_TAG_SHELLY, "HTTP Error " + String(code) + " when contacting Shelly");
        Logger::getInstance().log(LOG_TAG_SHELLY, api);
        http.end();
        return;
    }

    String payload = http.getString();
    http.end();

    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Logger::getInstance().log(LOG_TAG_SHELLY, "Failed parsing Shelly JSON: " + String(err.f_str()));
        return;
    }

    // Shelly 3EM returns an "emeters" array, each entry has "power", "energy", "voltage", "current" etc.
    // We'll read meter 0 by default.

    if (!doc.containsKey("emeters") || !doc["emeters"].is<JsonArray>()) {
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly JSON does not contain emeters array");
        return;
    }

    JsonArray emeters = doc["emeters"].as<JsonArray>();
    if (emeters.size() == 0) {
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly emeters array is empty");
        return;
    }

    JsonObject m = emeters[0];

    // Fill data. Some fields may be missing; default to 0.
    currentData.power_w = m.containsKey("power") ? m["power"].as<float>() : 0.0f;
    // Some firmwares report total in Wh, some energy in kWh. Use 'total' if present and convert from Wh to kWh.
    if (m.containsKey("total")) {
        currentData.energy_kwh = m["total"].as<float>() / 1000.0f;
    } else if (m.containsKey("energy")) {
        currentData.energy_kwh = m["energy"].as<float>();
    } else {
        currentData.energy_kwh = 0.0f;
    }
    currentData.voltage_v = m.containsKey("voltage") ? m["voltage"].as<float>() : 0.0f;
    currentData.current_a = m.containsKey("current") ? m["current"].as<float>() : 0.0f;
    // Shelly doesn't provide frequency/power_factor in some firmwares; set to 0 if absent
    currentData.frequency_hz = m.containsKey("frequency") ? m["frequency"].as<float>() : 50.0f;
    currentData.power_factor = m.containsKey("pf") ? m["pf"].as<float>() : 0.0f;

    // Ensure sensible ranges
    if (currentData.voltage_v <= 0 || currentData.current_a < 0 || currentData.power_w < 0) {
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly returned invalid measurements");
        return;
    }

    bOnline = true;
}

String ShellyClient::getDataJson() {
    String json = "{";
    json += "\"Power (Kw)\":" + String(currentData.power_w / 1000.0f, 3);
    json += ",\"Energy (Kwh)\":" + String(currentData.energy_kwh, 3);
    json += ",\"Voltage (V)\":" + String(currentData.voltage_v, 1);
    json += ",\"Current (A)\":" + String(currentData.current_a, 2);
    json += ",\"Frequency (Hz)\":" + String(currentData.frequency_hz, 1);
    json += ",\"Power Factor\":" + String(currentData.power_factor, 2);
    json += "}";
    return json;
}
