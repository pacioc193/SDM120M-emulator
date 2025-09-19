#ifndef SHELLY_CLIENT_H
#define SHELLY_CLIENT_H

#pragma once

#include <Arduino.h>
#include "DataManager.h"
#include "ConfigManager.h"

class ShellyClient : public DataManager {
public:
    void fetchAllData(ConfigManager& config) override;
    String getDataJson();
    MeterData getCurrentData() const { return currentData; }
    bool isOnline() const { return bOnline; }

    // Telemetry for diagnostics
    int lastHttpCode = 0;
    String lastError; // last error message
    unsigned long lastSuccessTs = 0; // epoch millis when last successful fetch occurred

    // Implement diagnostic interface
    int getLastHttpCode() const override { return lastHttpCode; }
    String getLastError() const override { return lastError; }
    unsigned long getLastSuccessTs() const override { return lastSuccessTs; }
};

#endif
