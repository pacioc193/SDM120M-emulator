#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#pragma once

#include "ConfigManager.h"
#include <Arduino.h>

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

    // Diagnostics interface
    virtual int getLastHttpCode() const { return 0; }
    virtual String getLastError() const { return String(); }
    virtual unsigned long getLastSuccessTs() const { return 0; }
    virtual unsigned long getLastSuccessAgeMs() const {
        unsigned long ts = getLastSuccessTs();
        if (ts == 0) return (unsigned long)(-1);
        unsigned long now = millis();
        return now >= ts ? (now - ts) : 0;
    }

protected:
    MeterData currentData;
    bool bOnline = false;

};

#endif // DATAMANAGER_H
