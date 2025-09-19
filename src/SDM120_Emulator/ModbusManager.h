// ModbusManager.h
#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <ModbusRTU.h>
#include <HardwareSerial.h>
#include "HomeAssistantClient.h" // To get data from HA

// Define Modbus Holding Register addresses for SDM120 simulation
// Values are floats, so they occupy two 16-bit registers.
// These addresses mimic some common SDM120 register mappings (simplified).
#define SDM120_REG_VOLTAGE          0x0000 // 2 registers for float
#define SDM120_REG_CURRENT          0x0006 // 2 registers for float
#define SDM120_REG_ACTIVE_POWER     0x000C // 2 registers for float
#define SDM120_REG_APPARENT_POWER   0x0012 // 2 registers for float
#define SDM120_REG_REACTIVE_POWER   0x0018 // 2 registers for float
#define SDM120_REG_POWER_FACTOR     0x001E // 2 registers for float
#define SDM120_REG_PHASE_ANGLE      0x0024 // 2 registers for float
#define SDM120_REG_FREQUENCY        0x0046 // 2 registers for float
#define SDM120_REG_TOTAL_ACTIVE_ENERGY 0x0048 // 2 registers for float (kWh)

// Modbus will consider the data source offline only after this timeout
#define MODBUS_OFFLINE_TIMEOUT_MS 30000UL // 30 seconds

class ModbusManager {
public:
    void begin(DataManager& client, uint8_t slaveId, int rxPin, int txPin, int deRePin = -1, int iBoud = 2400);
    void modbus_loop();

private:
    ModbusRTU mb;
    DataManager* pxClient;

    // timestamp of last successful data availability
    unsigned long lastGoodMillis = 0;

    // Union to convert float to two uint16_t for Modbus registers
    typedef union {
        float f;
        uint16_t i[2];
    } float_to_uint16;

    void updateModbusRegisters();
};

#endif // MODBUS_MANAGER_H