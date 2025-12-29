#include "ShellyClient.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Logger.h"
#include "WiFi.h"

void ShellyClient::fetchAllData(ConfigManager& config) {
    bOnline = false;
    lastError = "";
    lastHttpCode = 0;

    if (WiFi.status() != WL_CONNECTED) {
        lastError = "WiFi not connected";
        Logger::getInstance().log(LOG_TAG_SHELLY, "WiFi not connected, cannot fetch data from Shelly.");
        return;
    }

    // Prefer explicit shelly_url config, fall back to home_assistant_url for compatibility
    String url = String(config.currentConfig.shelly_url);
    if (url.isEmpty()) {
        url = String(config.currentConfig.home_assistant_url);
    }

    if (url.isEmpty()) {
        lastError = "Shelly URL not configured";
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly URL not found...");
        return;
    }

    // Get channel from config
    int channel = config.currentConfig.shelly_channel;
    if (channel < 0) channel = 0;
    if (channel > 2) channel = 2;

    // Shelly 3EM status endpoint typically at /status
    String api = url + "/status";

    HTTPClient http;
    http.begin(api);
    http.setTimeout(2000);
    int code = http.GET();
    lastHttpCode = code;
    if (code != HTTP_CODE_OK) {
        lastError = "HTTP Error " + String(code);
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
        lastError = "JSON parse error: " + String(err.f_str());
        Logger::getInstance().log(LOG_TAG_SHELLY, "Failed parsing Shelly JSON: " + String(err.f_str()));
        return;
    }

    // Shelly 3EM returns an "emeters" array, each entry has "power", "energy", "voltage", "current" etc.
    // Use the configured channel.

    if (!doc.containsKey("emeters") || !doc["emeters"].is<JsonArray>()) {
        lastError = "emeters array missing";
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly JSON does not contain emeters array");
        return;
    }

    JsonArray emeters = doc["emeters"].as<JsonArray>();
    if (emeters.size() == 0) {
        lastError = "emeters array empty";
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly emeters array is empty");
        return;
    }

    // Use configured channel, fallback to 0 if out of bounds
    if ((size_t)channel >= emeters.size()) {
        channel = 0;
    }

    JsonObject m = emeters[channel];

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
        lastError = "invalid measurements";
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly returned invalid measurements");
        return;
    }

    bOnline = true;
    lastSuccessTs = millis();
}

String ShellyClient::getDataJson() {
    // Format values according to requirements:
    // power_kw 3 decimals, energy_kwh 0 decimals, voltage 2, current 2, frequency 2, power factor 2
    String power_kw = String(currentData.power_w / 1000.0f, 3);
    String energy_kwh = String(currentData.energy_kwh, 0);
    String voltage_v = String(currentData.voltage_v, 2);
    String current_a = String(currentData.current_a, 2);
    String frequency_hz = String(currentData.frequency_hz, 2);
    String power_factor = String(currentData.power_factor, 2);

    StaticJsonDocument<256> doc;
    doc["Power (Kw)"] = power_kw;
    doc["Energy (Kwh)"] = energy_kwh;
    doc["Voltage (V)"] = voltage_v;
    doc["Current (A)"] = current_a;
    doc["Frequency (Hz)"] = frequency_hz;
    doc["Power Factor"] = power_factor;

    String out;
    serializeJson(doc, out);
    return out;
}
