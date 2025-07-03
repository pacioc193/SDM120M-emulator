#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <EEPROM.h>
#include <Arduino.h>

struct Config {
    char ssid[32];
    char password[64];
    char home_assistant_url[64];
    char home_assistant_token[256];
    char home_assistant_entity_power[64];
    char home_assistant_entity_voltage[64];
    char home_assistant_entity_current[64];
    char home_assistant_entity_energy[64];
    char home_assistant_entity_frequency[64];
    char home_assistant_entity_power_factor[64];
    bool configured;
};

class ConfigManager {
public:
    Config currentConfig;

    void begin();
    void load();
    void save();
    void reset();

private:
    const int EEPROM_SIZE = sizeof(Config);
    void setDefaultConfig();
};

#endif