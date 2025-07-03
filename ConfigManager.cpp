#include "ConfigManager.h"
#include "Logger.h"

void ConfigManager::begin() {
    EEPROM.begin(EEPROM_SIZE);
    load();
}

void ConfigManager::load() {
    EEPROM.get(0, currentConfig);

    // Sanitize strings
    currentConfig.ssid[sizeof(currentConfig.ssid) - 1] = '\0';
    currentConfig.password[sizeof(currentConfig.password) - 1] = '\0';
	currentConfig.home_assistant_entity_power[sizeof(currentConfig.home_assistant_entity_power) - 1] = '\0';
	currentConfig.home_assistant_entity_energy[sizeof(currentConfig.home_assistant_entity_energy) - 1] = '\0';
	currentConfig.home_assistant_entity_voltage[sizeof(currentConfig.home_assistant_entity_voltage) - 1] = '\0';
	currentConfig.home_assistant_entity_current[sizeof(currentConfig.home_assistant_entity_current) - 1] = '\0';
	currentConfig.home_assistant_entity_frequency[sizeof(currentConfig.home_assistant_entity_frequency) - 1] = '\0';
    currentConfig.home_assistant_entity_power_factor[sizeof(currentConfig.home_assistant_entity_power_factor) - 1] = '\0';
	currentConfig.home_assistant_token[sizeof(currentConfig.home_assistant_token) - 1] = '\0';
	currentConfig.home_assistant_url[sizeof(currentConfig.home_assistant_url) - 1] = '\0';

    // Load configuration from EEPROM
    if (!currentConfig.configured || strlen(currentConfig.ssid) == 0) {
        Logger::getInstance().log(LOG_TAG_CONFIG, "No valid configuration found. Loading default values.");
        setDefaultConfig();
    }
    else {
        Logger::getInstance().log(LOG_TAG_CONFIG, "Configuration loaded from EEPROM for SSID: " + String(currentConfig.ssid));
    }
}

void ConfigManager::save() {
    currentConfig.configured = true;
    EEPROM.put(0, currentConfig);
    EEPROM.commit();
    Logger::getInstance().log(LOG_TAG_CONFIG, "Configuration saved to EEPROM.");
}

void ConfigManager::reset() {
    setDefaultConfig();
    save();
    Logger::getInstance().log(LOG_TAG_CONFIG, "Configuration reset to default values.");
}

void ConfigManager::setDefaultConfig() {
    memset(&currentConfig, 0, sizeof(Config)); // Clear the structure
    currentConfig.configured = false;
}