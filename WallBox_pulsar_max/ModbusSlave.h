#ifndef MODBUSSLAVE_H
#define MODBUSSLAVE_H

#include <ModbusRTU.h>

struct MeterData {
    float voltage;      // Volts
    float current;      // Amps
    float active_power; // Watts
    float apparent_power; // VA
    float reactive_power; // VAR
    float pf;           // 0.0 - 1.0
    float frequency;    // Hz
    float total_energy; // kWh
};

class ModbusSlaveManager {
public:
    void begin();
    void loop();
    void updateData(const MeterData& data);

private:
    ModbusRTU _mb;
    
    // Helpers
    void setInt32(uint16_t start_reg, int32_t value);
    void setInt16(uint16_t start_reg, int16_t value);
    void setUint16(uint16_t start_reg, uint16_t value);
};

#endif
