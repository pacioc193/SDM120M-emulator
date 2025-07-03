#ifndef HOME_ASSISTANT_CLIENT_H
#define HOME_ASSISTANT_CLIENT_H

#pragma once

#include <Arduino.h>
#include "ConfigManager.h"

struct HAData {
    float power_w = 0.0;
    float energy_kwh = 0.0;
    float voltage_v = 0.0;
    float current_a = 0.0;
    float frequency_hz = 0.0;
    float power_factor = 0.0;
};

class HomeAssistantClient {
public:
    HAData currentData;
    void fetchAllData(ConfigManager& config);
    String getDataJson();
    HAData getCurrentData() const { return currentData; }
    bool isOnline() const { return haOnline; }

private:
    float getHAEntityState(const String& url, const String& token, const String& entity_id);
    bool haOnline = false;
};

#endif