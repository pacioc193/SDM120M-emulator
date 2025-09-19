#ifndef HOME_ASSISTANT_CLIENT_H
#define HOME_ASSISTANT_CLIENT_H

#pragma once

#include <Arduino.h>
#include "DataManager.h"


class HomeAssistantClient : public DataManager {
public:
    void fetchAllData(ConfigManager& config);
    String getDataJson();
    MeterData getCurrentData() const { return currentData; }
    bool isOnline() const { return bOnline; }

private:
    float getHAEntityState(const String& url, const String& token, const String& entity_id);
};

#endif