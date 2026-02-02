#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Wallbox Emulator Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; max-width: 600px; }
    input, select { width: 100%; padding: 8px; margin: 5px 0; box-sizing: border-box; }
    label { font-weight: bold; }
    .card { border: 1px solid #ccc; padding: 15px; border-radius: 5px; margin-bottom: 15px; }
    .btn { background-color: #007bff; color: white; padding: 10px; border: none; cursor: pointer; width: 100%; }
    .btn:hover { background-color: #0056b3; }
    #log { height: 200px; overflow-y: scroll; background: #f0f0f0; border: 1px solid #999; padding: 5px; font-family: monospace; font-size: 12px; }
  </style>
</head>
<body>
  <h2>Wallbox Emulator Setup</h2>
  
  <form action="/save" method="POST">
    <div class="card">
      <h3>WiFi Settings</h3>
      <label>SSID</label><input type="text" name="ssid" id="ssid">
      <label>Password</label><input type="password" name="pass" id="pass">
    </div>

    <div class="card">
      <h3>MQTT (Primary)</h3>
      <label>Broker IP</label><input type="text" name="m_server" id="m_server">
      <label>Port</label><input type="number" name="m_port" id="m_port" value="1883">
      <label>User</label><input type="text" name="m_user" id="m_user">
      <label>Pass</label><input type="password" name="m_pass" id="m_pass">
      <label>Topic</label><input type="text" name="m_topic" id="m_topic" placeholder="home/energy">
    </div>

    <div class="card">
      <h3>Shelly (Failover)</h3>
      <label>IP Address</label><input type="text" name="s_ip" id="s_ip">
      <label>Channel</label>
      <select name="s_chan" id="s_chan">
        <option value="0">Channel 0 (EM0)</option>
        <option value="1">Channel 1 (EM1)</option>
        <option value="2">Channel 2 (EM2)</option>
      </select>
    </div>

    <button type="submit" class="btn">Save & Reboot</button>
  </form>

  <div class="card">
    <h3>Live Logs</h3>
    <div id="log"></div>
  </div>

  <script>
    // Fill form with current values
    fetch('/config.json').then(res => res.json()).then(data => {
      document.getElementById('ssid').value = data.ssid || '';
      document.getElementById('pass').value = data.pass || '';
      document.getElementById('m_server').value = data.m_server || '';
      document.getElementById('m_port').value = data.m_port || 1883;
      document.getElementById('m_user').value = data.m_user || '';
      document.getElementById('m_pass').value = data.m_pass || '';
      document.getElementById('m_topic').value = data.m_topic || '';
      document.getElementById('s_ip').value = data.s_ip || '';
      document.getElementById('s_chan').value = data.s_chan || 0;
    });

    // WebSocket Log
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onmessage = function(event) {
        var log = document.getElementById('log');
        log.innerHTML += event.data + '<br>';
        log.scrollTop = log.scrollHeight;
      };
      websocket.onclose = function() { setTimeout(initWebSocket, 2000); };
    }
    window.onload = initWebSocket;
  </script>
</body>
</html>
)rawliteral";

#endif
