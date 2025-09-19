#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#pragma once

#include "ConfigManager.h"

struct MeterData
{
    float power_w = 0.0f;
    float energy_kwh = 0.0f;
    float voltage_v = 0.0f;
    float current_a = 0.0f;
    float frequency_hz = 0.0f;
    float power_factor = 0.0f;
};

class DataManager
{
public:
    virtual void fetchAllData(ConfigManager& config) = 0;
    virtual MeterData getCurrentData() const = 0;
	virtual bool isOnline() const = 0;
    virtual String getDataJson();
    virtual ~DataManager() {}
protected:
    MeterData currentData;
    bool bOnline = false;

};

#endif // DATAMANAGER_H
