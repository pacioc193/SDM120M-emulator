#include "Config.h"

ConfigManager Config;

void ConfigManager::begin() {
    _prefs.begin("wb_emulator", false); // Namespace "wb_emulator"
    load();
}

void ConfigManager::load() {
    _config.wifi_ssid = _prefs.getString("w_ssid", "");
    _config.wifi_pass = _prefs.getString("w_pass", "");

    _config.mqtt_server = _prefs.getString("m_server", "");
    _config.mqtt_port = _prefs.getInt("m_port", 1883);
    _config.mqtt_user = _prefs.getString("m_user", "");
    _config.mqtt_pass = _prefs.getString("m_pass", "");
    _config.mqtt_topic = _prefs.getString("m_topic", "home/energy");

    _config.shelly_ip = _prefs.getString("s_ip", "");
    _config.shelly_channel = _prefs.getInt("s_chan", 0);
}

void ConfigManager::save(const SystemConfig& config) {
    _config = config;
    _prefs.putString("w_ssid", _config.wifi_ssid);
    _prefs.putString("w_pass", _config.wifi_pass);

    _prefs.putString("m_server", _config.mqtt_server);
    _prefs.putInt("m_port", _config.mqtt_port);
    _prefs.putString("m_user", _config.mqtt_user);
    _prefs.putString("m_pass", _config.mqtt_pass);
    _prefs.putString("m_topic", _config.mqtt_topic);

    _prefs.putString("s_ip", _config.shelly_ip);
    _prefs.putInt("s_chan", _config.shelly_channel);
}

SystemConfig ConfigManager::get() const {
    return _config;
}
