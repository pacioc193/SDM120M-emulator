#ifndef SHELLY_GEN2_CLIENT_H
#define SHELLY_GEN2_CLIENT_H

#pragma once

#include <Arduino.h>
#include "DataManager.h"
#include "ConfigManager.h"

// Client for Shelly 3EM Gen 2/3 devices using the Gen2+ RPC API
class ShellyGen2Client : public DataManager {
public:
    void fetchAllData(ConfigManager& config) override;
    String getDataJson() override;
    MeterData getCurrentData() const override { return currentData; }
    bool isOnline() const override { return bOnline; }

    // Telemetry for diagnostics
    int lastHttpCode = 0;
    String lastError;
    unsigned long lastSuccessTs = 0;

    // Implement diagnostic interface
    int getLastHttpCode() const override { return lastHttpCode; }
    String getLastError() const override { return lastError; }
    unsigned long getLastSuccessTs() const override { return lastSuccessTs; }
};

#endif
