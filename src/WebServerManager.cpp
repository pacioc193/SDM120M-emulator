#include "WebServerManager.h"
#include "Logger.h"

String WebServerManager::generateIndexHtml() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 SDM120 Emulator</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; margin: 0; background-color: #f0f2f5; color: #333; }
        .container { max-width: 800px; margin: auto; background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 4px 12px rgba(0,0,0,0.1); }
        h1, h2 { color: #1c1e21; }
        h1 { text-align: center; margin-bottom: 20px;}

        /* Stile Tab migliorato */
        .tab { overflow: hidden; border-bottom: 1px solid #ddd; background-color: #f0f2f5; border-radius: 6px; }
        .tab button { background-color: inherit; float: left; border: none; outline: none; cursor: pointer; padding: 14px 16px; transition: background-color 0.3s, color 0.3s; font-size: 16px; color: #606770; font-weight: 500;}
        .tab button:hover { background-color: #e4e6e9; }
        .tab button.active { color: #0056b3; border-bottom: 3px solid #0056b3; font-weight: 600; }
        
        .tabcontent { display: none; padding: 20px; border-top: none; animation: fadeIn 0.5s; }
        @keyframes fadeIn { from {opacity: 0;} to {opacity: 1;} }

        .data-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; }
        .data-item { background-color: #f0f2f5; padding: 15px; border-radius: 8px; text-align: center; }
        .data-label { font-size: 0.9em; color: #606770; }
        .data-value { font-size: 1.5em; font-weight: bold; color: #1c1e21; }
        form { display: flex; flex-direction: column; }
        label { margin: 10px 0 5px; font-weight: bold; }
        input, select, button { padding: 12px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 6px; font-size: 1em; }
        button { background-color: #007bff; color: white; border: none; cursor: pointer; font-weight: bold; }
        button:hover { background-color: #0056b3; }
        .status-message { text-align: center; padding: 10px; margin-bottom: 15px; font-size: 1.1em; border-radius: 6px; }
        .connected { color: #28a745; background-color: #e9f7ef; } 
        .not-connected { color: #dc3545; background-color: #fbebed; }
        #log_table { width: 100%; border-collapse: collapse; margin-top: 10px;}
        #log_table th, #log_table td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        #log_table th { background-color: #f2f2f2; }
        .log-controls { display: flex; justify-content: space-between; align-items: center; }
    </style>
</head>
<body>
    <div class="container">
        <h1>SDM120 Emulator</h1>
        <div class="status-message" id="wifiStatus">Checking...</div>
        
        <div class="tab">
            <button class="tablinks" onclick="openTab(event, 'Dashboard')" id="defaultOpen">Dashboard</button>
            <button class="tablinks" onclick="openTab(event, 'Config')">Configuration</button>
            <button class="tablinks" onclick="openTab(event, 'Logs')">Logs</button>
        </div>

        <div id="Dashboard" class="tabcontent">
            <h2>Live Data</h2>
            <div class="data-grid" id="dashboard-data">
                </div>
        </div>

        <div id="Config" class="tabcontent">
            <h2>Wi-Fi Configuration</h2>
            <form onsubmit="saveWifi(event)">
                <label for="ssid">Select Network (SSID):</label>
                <select id="ssid" name="ssid" required>
                    <option value="%CURRENT_SSID_OPTION_VALUE%" selected>%CURRENT_SSID_DISPLAY_TEXT%</option>
                </select>
                <label for="password">Password:</label>
                <input type="password" id="password" name="password" value="%CURRENT_PASSWORD%">
                <button type="submit">Save Wi-Fi Settings</button>
                <button type="button" onclick="requestWifiScan()">Scan Networks</button>
                <div id="scanResults" style="margin-top: 10px;"></div>
            </form>

            <hr style="margin: 30px 0;">

            <h2>Home Assistant Configuration</h2>
            <form onsubmit="saveHomeAssistant(event)">
                <label for="ha_url">Home Assistant URL:</label>
                <input type="text" id="ha_url" name="ha_url" value="%HA_URL%">
                <label for="ha_token">Long-Lived Access Token:</label>
                <input type="text" id="ha_token" name="ha_token" value="%HA_TOKEN%">
                <label for="ha_power_id">Power Entity ID:</label>
                <input type="text" id="ha_entity_power" name="ha_entity_power" value="%HA_POWER_ID%">
                <label for="ha_voltage_id">Voltage Entity ID:</label>
                <input type="text" id="ha_entity_voltage" name="ha_entity_voltage" value="%HA_VOLTAGE_ID%">
                <label for="ha_current_id">Current Entity ID:</label>
                <input type="text" id="ha_entity_current" name="ha_entity_current" value="%HA_CURRENT_ID%">
                <label for="ha_energy_id">Energy Entity ID:</label>
                <input type="text" id="ha_entity_energy" name="ha_entity_energy" value="%HA_ENERGY_ID%">
                <label for="ha_frequency_id">Frequency Entity ID:</label>
                <input type="text" id="ha_entity_frequency" name="ha_entity_frequency" value="%HA_FREQUENCY_ID%">
                <label for="ha_power_factor_id">Power Factor Entity ID:</label>
                <input type="text" id="ha_entity_power_factor" name="ha_entity_power_factor" value="%HA_POWER_FACTOR_ID%">
                <button type="submit">Save Home Assistant Settings</button>
            </form>
        </div>

        <div id="Logs" class="tabcontent">
            <h2>System Log</h2>
            <div class="log-controls">
                <div>
                    <label for="log_filter" style="margin:0; margin-right: 10px;">Filter by Tag:</label>
                    <select id="log_filter" onchange="updateLogs()">
                        <option value="All">All</option>
                        <option value="Generic">Generic</option>
                        <option value="ServerWeb">ServerWeb</option>
                        <option value="WiFi">WiFi</option>
                        <option value="HomeAssistant">HomeAssistant</option>
                        <option value="Config">Config</option>
                        <option value="Modbus">Modbus</option>
                    </select>
                </div>
                <a href="/log.txt" download="esp32_log.txt"><button type="button">Download Log (.txt)</button></a>
            </div>
            <div style="max-height: 400px; overflow-y: auto; margin-top: 10px;">
                <table id="log_table">
                    <thead><tr><th>Timestamp</th><th>Tag</th><th>Message</th></tr></thead>
                    <tbody id="log_body"></tbody>
                </table>
            </div>
        </div>

    </div>

    <script>
        let logInterval = null;

        function openTab(evt, tabName) {
            var i, tabcontent, tablinks;
            tabcontent = document.getElementsByClassName("tabcontent");
            for (i = 0; i < tabcontent.length; i++) {
                tabcontent[i].style.display = "none";
            }
            tablinks = document.getElementsByClassName("tablinks");
            for (i = 0; i < tablinks.length; i++) {
                tablinks[i].className = tablinks[i].className.replace(" active", "");
            }
            document.getElementById(tabName).style.display = "block";
            evt.currentTarget.className += " active";
            if(tabName === 'Logs') {
                updateLogs();
                if (!logInterval) {
                    logInterval = setInterval(updateLogs, 1000);
                }
            } else {
                if (logInterval) {
                    clearInterval(logInterval);
                    logInterval = null;
                }
            }
            if(tabName === 'Dashboard') updateData(); // Ensure data is updated when dashboard is opened
            if(tabName === 'Config') requestWifiScan(); // Scan for networks when Config tab is opened
        }
        
        function updateWifiStatus() {
            fetch('/wifistatus')
                .then(response => response.text())
                .then(status => {
                    const wifiStatusDiv = document.getElementById('wifiStatus');
                    wifiStatusDiv.innerText = status;
                    if (status.includes("Connected")) {
                        wifiStatusDiv.className = "status-message connected";
                    } else {
                        wifiStatusDiv.className = "status-message not-connected";
                    }
                })
                .catch(error => console.error('Error fetching Wi-Fi status:', error));
        }

        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    const dashboard = document.getElementById('dashboard-data');
                    dashboard.innerHTML = ''; // Clear previous data
                    for (const key in data) {
                        const item = document.createElement('div');
                        item.className = 'data-item';
                        item.innerHTML = `<div class="data-label">${key.replace(/_/g, ' ')}</div><div class="data-value">${data[key]}</div>`;
                        dashboard.appendChild(item);
                    }
                })
                .catch(error => console.error('Error fetching data:', error));
        }

        function updateLogs() {
            const filter = document.getElementById('log_filter').value;
            fetch('/log_html?tag=' + filter).then(r => r.text()).then(html => {
                document.getElementById('log_body').innerHTML = html;
            }).catch(e => console.error('Error logs:', e));
        }

        function requestWifiScan() {
            const ssidSelect = document.getElementById('ssid');
            const scanResultsDiv = document.getElementById('scanResults');
            ssidSelect.innerHTML = '<option value="">Scanning...</option>'; // Clear and show scanning message
            scanResultsDiv.innerHTML = 'Scanning for networks...';

            fetch('/scan')
                .then(response => response.json())
                .then(networks => {
                    ssidSelect.innerHTML = ''; // Clear options
                    let currentSSID = "%CURRENT_SSID_FOR_JS%"; // Placeholder for current SSID from C++
                    let foundCurrent = false;

                    // Add an empty/default option
                    ssidSelect.innerHTML += '<option value="">-- Select Network --</option>';

                    networks.forEach(net => {
                        const option = document.createElement('option');
                        option.value = net.ssid;
                        option.textContent = `${net.ssid} (Signal: ${net.rssi} dBm)`;
                        if (net.ssid === currentSSID) {
                            option.selected = true;
                            foundCurrent = true;
                        }
                        ssidSelect.appendChild(option);
                    });

                    if (currentSSID && !foundCurrent) {
                        // If the current SSID wasn't found in the scan, add it to the list
                        const option = document.createElement('option');
                        option.value = currentSSID;
                        option.textContent = `${currentSSID} (Not in current scan)`;
                        option.selected = true;
                        ssidSelect.appendChild(option);
                    }

                    scanResultsDiv.innerHTML = 'Scan complete.';
                })
                .catch(error => {
                    console.error('Error scanning for networks:', error);
                    ssidSelect.innerHTML = '<option value="">Error scanning</option>';
                    scanResultsDiv.innerHTML = 'Error scanning for networks.';
                });
        }


        function saveWifi(event) {
            event.preventDefault();
            const formData = new URLSearchParams(new FormData(event.target)).toString();
            fetch('/savewifi', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData
            })
            .then(response => response.text())
            .then(data => {
                alert(data);
                // ESP will reboot, no further action needed here
            })
            .catch(error => console.error('Error saving Wi-Fi:', error));
        }

        function saveHomeAssistant(event) {
            event.preventDefault();
            const formData = new URLSearchParams(new FormData(event.target)).toString();
            fetch('/saveha', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData
            })
            .then(response => response.text())
            .then(data => {
                alert(data);
            })
            .catch(error => console.error('Error saving Home Assistant config:', error));
        }
        
        document.getElementById("defaultOpen").click(); // Open Dashboard by default
        setInterval(updateData, 500); // Update dashboard data every 0.5 seconds
        setInterval(updateWifiStatus, 5000); // Update Wi-Fi status every 5 seconds
        updateWifiStatus(); // Initial call for Wi-Fi status
    </script>
</body>
</html>
)rawliteral";

    html.replace("%HA_URL%", String(pConfig->currentConfig.home_assistant_url));
    html.replace("%HA_TOKEN%", String(pConfig->currentConfig.home_assistant_token));
    html.replace("%HA_POWER_ID%", String(pConfig->currentConfig.home_assistant_entity_power));
    html.replace("%HA_VOLTAGE_ID%", String(pConfig->currentConfig.home_assistant_entity_voltage));
    html.replace("%HA_CURRENT_ID%", String(pConfig->currentConfig.home_assistant_entity_current));
    html.replace("%HA_ENERGY_ID%", String(pConfig->currentConfig.home_assistant_entity_energy));
    html.replace("%HA_FREQUENCY_ID%", String(pConfig->currentConfig.home_assistant_entity_frequency));
    html.replace("%HA_POWER_FACTOR_ID%", String(pConfig->currentConfig.home_assistant_entity_power_factor));

    // Placeholders for current Wi-Fi configuration (for the dropdown and JS)
    String currentSsid = String(pConfig->currentConfig.ssid);
    html.replace("%CURRENT_SSID_OPTION_VALUE%", currentSsid);
    html.replace("%CURRENT_SSID_DISPLAY_TEXT%", currentSsid.isEmpty() ? "-- No Network Set --" : currentSsid);
    html.replace("%CURRENT_SSID_FOR_JS%", currentSsid); // For JavaScript to check
    html.replace("%CURRENT_PASSWORD%", ""); // Password is not displayed for security reasons.
    return html;
}

void WebServerManager::begin(ConfigManager& config, WiFiManager& wifi, HomeAssistantClient& ha) {
	pConfig = &config;
	pWifi = &wifi;
	pHa = &ha;
	setupRoutes();
	server.begin();
	Logger::getInstance().log(LOG_TAG_SERVERWEB, "Server Web is running.");
}

void WebServerManager::handleClient() {
	server.handleClient();
}

void WebServerManager::setupRoutes() {
	server.on("/", HTTP_GET, [this]() { this->handleRoot(); });
	server.on("/data", HTTP_GET, [this]() { this->handleData(); });
	server.on("/wifistatus", HTTP_GET, [this]() { this->handleWifiStatus(); });
	server.on("/scan", HTTP_GET, [this]() { this->handleScan(); });
	server.on("/savewifi", HTTP_POST, [this]() { this->handleSaveWifi(); });
	server.on("/saveha", HTTP_POST, [this]() { this->handleSaveHomeAssistant(); });
	server.on("/log_html", HTTP_GET, [this]() { this->handleLogHtml(); });
	server.on("/log.txt", HTTP_GET, [this]() { this->handleLogTxt(); });
	server.onNotFound([this]() { this->handleNotFound(); });
}

void WebServerManager::handleRoot() {
	server.send(200, "text/html", generateIndexHtml());
}

void WebServerManager::handleData() {
	server.send(200, "application/json", pHa->getDataJson());
}

void WebServerManager::handleWifiStatus() {
	server.send(200, "text/plain", pWifi->getStatusString());
}

void WebServerManager::handleScan() {
	server.send(200, "application/json", pWifi->scanNetworks());
}

void WebServerManager::handleLogTxt() {
	server.send(200, "text/plain", Logger::getInstance().getLogsTxt());
}

void WebServerManager::handleSaveWifi() {
	if (server.hasArg("ssid")) {
		server.arg("ssid").toCharArray(pConfig->currentConfig.ssid, sizeof(pConfig->currentConfig.ssid));
		server.arg("password").toCharArray(pConfig->currentConfig.password, sizeof(pConfig->currentConfig.password));
		pConfig->save();

		String response = "Wi-Fi Saved!\nReboot in progress... ESP will be connected to " + server.arg("ssid") + ".";
		server.send(200, "text/html", response);
		Logger::getInstance().log(LOG_TAG_SERVERWEB, "Wi-Fi configuration saved for SSID: " + server.arg("ssid") + ". Rebooting...");
		delay(2000);
		ESP.restart();
	}
	else {
		server.send(400, "text/plain", "Error: SSID not provided.");
	}
}

void WebServerManager::handleSaveHomeAssistant() {
	server.arg("ha_url").toCharArray(pConfig->currentConfig.home_assistant_url, sizeof(pConfig->currentConfig.home_assistant_url));
	server.arg("ha_token").toCharArray(pConfig->currentConfig.home_assistant_token, sizeof(pConfig->currentConfig.home_assistant_token));
	server.arg("ha_entity_power").toCharArray(pConfig->currentConfig.home_assistant_entity_power, sizeof(pConfig->currentConfig.home_assistant_entity_power));
	server.arg("ha_entity_voltage").toCharArray(pConfig->currentConfig.home_assistant_entity_voltage, sizeof(pConfig->currentConfig.home_assistant_entity_voltage));
	server.arg("ha_entity_current").toCharArray(pConfig->currentConfig.home_assistant_entity_current, sizeof(pConfig->currentConfig.home_assistant_entity_current));
	server.arg("ha_entity_energy").toCharArray(pConfig->currentConfig.home_assistant_entity_energy, sizeof(pConfig->currentConfig.home_assistant_entity_energy));
	server.arg("ha_entity_frequency").toCharArray(pConfig->currentConfig.home_assistant_entity_frequency, sizeof(pConfig->currentConfig.home_assistant_entity_frequency));
	server.arg("ha_entity_power_factor").toCharArray(pConfig->currentConfig.home_assistant_entity_power_factor, sizeof(pConfig->currentConfig.home_assistant_entity_power_factor));

	pConfig->save();

	Logger::getInstance().log(LOG_TAG_SERVERWEB, "Home Assistant Configuration Saved.");
	String response = "Home Assistant Configuration Saved!";
	server.send(200, "text/html", response);
}

void WebServerManager::handleNotFound() {
	server.send(404, "text/plain", "404: Not Found");
}

void WebServerManager::handleLogHtml() {
    String tagFilter = "All";
    if (server.hasArg("tag")) {
        tagFilter = server.arg("tag");
    }
    server.send(200, "text/plain", Logger::getInstance().getLogsHtml(tagFilter));
}