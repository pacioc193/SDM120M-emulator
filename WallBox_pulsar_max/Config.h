#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

struct SystemConfig {
    // WiFi
    String wifi_ssid;
    String wifi_pass;

    // MQTT (Primary)
    String mqtt_server;
    int mqtt_port;
    String mqtt_user;
    String mqtt_pass;
    String mqtt_topic;

    // Shelly (Failover)
    String shelly_ip;
    int shelly_channel; // 0, 1, 2
};

class ConfigManager {
public:
    void begin();
    void load();
    void save(const SystemConfig& config);
    SystemConfig get() const;

private:
    Preferences _prefs;
    SystemConfig _config;
};

extern ConfigManager Config;

#endif
