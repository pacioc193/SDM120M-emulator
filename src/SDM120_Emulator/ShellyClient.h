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
};

#endif
