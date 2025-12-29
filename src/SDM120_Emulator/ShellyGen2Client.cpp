#include "ShellyGen2Client.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Logger.h"
#include "WiFi.h"

// Shelly 3EM Gen2/3 uses RPC API endpoints
// For triphase profile: GET /rpc/EM.GetStatus?id=0 returns combined 3-phase data
// For monophase profile: GET /rpc/EM1.GetStatus?id=<channel> returns per-channel data
// EMData.GetStatus / EM1Data.GetStatus returns energy totals

void ShellyGen2Client::fetchAllData(ConfigManager& config) {
    bOnline = false;
    lastError = "";
    lastHttpCode = 0;

    if (WiFi.status() != WL_CONNECTED) {
        lastError = "WiFi not connected";
        Logger::getInstance().log(LOG_TAG_SHELLY, "WiFi not connected, cannot fetch data from Shelly Gen2.");
        return;
    }

    String url = String(config.currentConfig.shelly_url);
    if (url.isEmpty()) {
        lastError = "Shelly URL not configured";
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly URL not found...");
        return;
    }

    // Remove trailing slash if present
    if (url.endsWith("/")) {
        url = url.substring(0, url.length() - 1);
    }

    int channel = config.currentConfig.shelly_channel;
    if (channel < 0) channel = 0;
    if (channel > 2) channel = 2;

    // Build API endpoint for Gen2/3 RPC - try EM.GetStatus first (triphase)
    String api = url + "/rpc/EM.GetStatus?id=0";

    HTTPClient http;
    http.begin(api);
    http.setTimeout(3000);
    int code = http.GET();
    lastHttpCode = code;

    bool isTriphase = true;

    if (code != HTTP_CODE_OK) {
        // Try EM1.GetStatus for monophase profile
        http.end();
        api = url + "/rpc/EM1.GetStatus?id=" + String(channel);
        http.begin(api);
        http.setTimeout(3000);
        code = http.GET();
        lastHttpCode = code;
        isTriphase = false;

        if (code != HTTP_CODE_OK) {
            lastError = "HTTP Error " + String(code);
            Logger::getInstance().log(LOG_TAG_SHELLY, "HTTP Error " + String(code) + " when contacting Shelly Gen2");
            Logger::getInstance().log(LOG_TAG_SHELLY, api);
            http.end();
            return;
        }
    }

    String payload = http.getString();
    http.end();

    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        lastError = "JSON parse error: " + String(err.f_str());
        Logger::getInstance().log(LOG_TAG_SHELLY, "Failed parsing Shelly Gen2 JSON: " + String(err.f_str()));
        return;
    }

    // Check if triphase (has "a_voltage") or monophase (has "voltage")
    // This overrides our earlier assumption based on endpoint success
    if (doc.containsKey("a_voltage")) {
        isTriphase = true;
    } else if (doc.containsKey("voltage")) {
        isTriphase = false;
    }

    if (isTriphase) {
        // Read from phase A, B or C based on channel
        String prefix;
        switch (channel) {
            case 0: prefix = "a_"; break;
            case 1: prefix = "b_"; break;
            case 2: prefix = "c_"; break;
            default: prefix = "a_"; break;
        }

        String voltKey = prefix + "voltage";
        String currKey = prefix + "current";
        String powerKey = prefix + "act_power";
        String pfKey = prefix + "pf";
        String freqKey = prefix + "freq";

        currentData.voltage_v = doc.containsKey(voltKey) ? doc[voltKey].as<float>() : 0.0f;
        currentData.current_a = doc.containsKey(currKey) ? doc[currKey].as<float>() : 0.0f;
        currentData.power_w = doc.containsKey(powerKey) ? doc[powerKey].as<float>() : 0.0f;
        currentData.power_factor = doc.containsKey(pfKey) ? doc[pfKey].as<float>() : 0.0f;
        currentData.frequency_hz = doc.containsKey(freqKey) ? doc[freqKey].as<float>() : 50.0f;
        currentData.energy_kwh = 0.0f;
    } else {
        // Monophase EM1.GetStatus response
        currentData.voltage_v = doc.containsKey("voltage") ? doc["voltage"].as<float>() : 0.0f;
        currentData.current_a = doc.containsKey("current") ? doc["current"].as<float>() : 0.0f;
        currentData.power_w = doc.containsKey("act_power") ? doc["act_power"].as<float>() : 0.0f;
        currentData.power_factor = doc.containsKey("pf") ? doc["pf"].as<float>() : 0.0f;
        currentData.frequency_hz = doc.containsKey("freq") ? doc["freq"].as<float>() : 50.0f;
        currentData.energy_kwh = 0.0f;
    }

    // Try to fetch energy from EMData or EM1Data
    String energyApi;
    if (isTriphase) {
        energyApi = url + "/rpc/EMData.GetStatus?id=0";
    } else {
        energyApi = url + "/rpc/EM1Data.GetStatus?id=" + String(channel);
    }

    http.begin(energyApi);
    http.setTimeout(2000);
    int energyCode = http.GET();
    if (energyCode == HTTP_CODE_OK) {
        String energyPayload = http.getString();
        StaticJsonDocument<512> energyDoc;
        if (!deserializeJson(energyDoc, energyPayload)) {
            // EMData/EM1Data response: { "id": 0, "total_act_energy": 12345.67, ... }
            if (energyDoc.containsKey("total_act_energy")) {
                currentData.energy_kwh = energyDoc["total_act_energy"].as<float>() / 1000.0f; // Wh to kWh
            } else if (energyDoc.containsKey("total_act")) {
                currentData.energy_kwh = energyDoc["total_act"].as<float>() / 1000.0f;
            }
        }
    }
    http.end();

    // Validate
    if (currentData.voltage_v <= 0) {
        lastError = "invalid voltage";
        Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly Gen2 returned invalid voltage");
        return;
    }

    bOnline = true;
    lastSuccessTs = millis();
    Logger::getInstance().log(LOG_TAG_SHELLY, "Shelly Gen2 fetch OK: V=" + String(currentData.voltage_v, 1) + " P=" + String(currentData.power_w, 1));
}

String ShellyGen2Client::getDataJson() {
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
