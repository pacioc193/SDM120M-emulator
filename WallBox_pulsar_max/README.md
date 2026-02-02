# Wallbox Pulsar Max Emulator (ESP32)

This firmware emulates a Carlo Gavazzi EM111 MV5 Energy Meter over Modbus RTU, allowing a Wallbox Pulsar Max to read energy data from Home Assistant (via MQTT) or a Shelly Gen3 device (Failover).

## Hardware
- **MCU**: ESP32 (DevKit V1 or similar)
- **RS485 Module**: MAX485 / RS485 TTL Adapter
  - **RX**: GPIO 16
  - **TX**: GPIO 17
  - **DE/RE**: GPIO 4 (Active HIGH)
  - **VCC/GND**: 3.3V or 5V (Check module spec)

## Software Dependencies
Install these libraries via Arduino Library Manager or Visual Micro:
1. **Modbus-ESP8266** by Andrii Emelianov (v4.1.0+)
2. **ArduinoJson** by Benoit Blanchon (v7.0.0+)
3. **PubSubClient** by Nick O'Leary (v2.8+)
4. **ESPAsyncWebServer** by Me-No-Dev (GitHub or manual install)
5. **AsyncTCP** by Me-No-Dev (Required for ESPAsyncWebServer)

## Configuration
1. Flash the firmware.
2. Connect to WiFi AP `Wallbox-Emulator-AP`.
3. Open `http://192.168.4.1`.
4. Configure WiFi, MQTT, and Shelly settings.
5. Save & Reboot.

## Features
- **Primary Source**: MQTT (`{"voltage": 230, "current": 10, "power": 2300, "energy": 1234.5, "pf": 0.98, "frequency": 50}`).
- **Failover**: Shelly Gen3 HTTP Polling.
- **Safety**: If all sources fail, Power/Current are set to 0 to pause charging.
- **Modbus**: Emulates EM111 MV5 registers with specific Endianness (Little Endian Word Swap for INT32).
- **OTA**: Supports wireless updates.
- **Logs**: Live WebSockets logging at `/`.

## Visual Studio / Visual Micro
- Open `WallBox_pulsar_max.ino` in Visual Studio with Visual Micro.
- Select your ESP32 board and COM port.
- Build and Upload.
