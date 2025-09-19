#ifndef HOME_ASSISTANT_CLIENT_H
#define HOME_ASSISTANT_CLIENT_H

#pragma once

#include <Arduino.h>
#include "DataManager.h"


class HomeAssistantClient : public DataManager {
public:
    void fetchAllData(ConfigManager& config) override;
    String getDataJson() override;
    MeterData getCurrentData() const { return currentData; }
    bool isOnline() const { return bOnline; }

    // Diagnostics
    int getLastHttpCode() const override { return lastHttpCode; }
    String getLastError() const override { return lastError; }
    unsigned long getLastSuccessTs() const override { return lastSuccessTs; }

private:
    float getHAEntityState(const String& url, const String& token, const String& entity_id);

    // Diagnostics fields
    int lastHttpCode = 0;
    String lastError;
    unsigned long lastSuccessTs = 0;
};

#endif