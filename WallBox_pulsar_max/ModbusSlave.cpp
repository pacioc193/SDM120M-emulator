#include "ModbusSlave.h"

// RS485 Pins
#define RX_PIN 16
#define TX_PIN 17
#define DE_RE_PIN 4

void ModbusSlaveManager::begin() {
    Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
    _mb.begin(&Serial1, DE_RE_PIN);
    _mb.slave(1); // Slave ID 1

    // Block 1: 0x0000 - 0x0011 (18 registers)
    // Covers Voltage, Current, Power, PF, Freq, Energy
    for (uint16_t i = 0; i <= 0x0011; i++) {
        _mb.addHreg(i, 0);
        _mb.addIreg(i, 0);
    }

    // Isolated registers
    _mb.addHreg(0x0302, 3); _mb.addIreg(0x0302, 3);   // FW Version
    _mb.addHreg(0x1002, 0); _mb.addIreg(0x1002, 0);   // System Config (1-Phase)
    _mb.addHreg(0x2000, 1); _mb.addIreg(0x2000, 1);   // Serial Addr

    // Static Value: Model ID at 0x000B
    _mb.Hreg(0x000B, 116);
    _mb.Ireg(0x000B, 116);
}

void ModbusSlaveManager::loop() {
    _mb.task();
}

// MV5 specific: Little Endian Word Swap for INT32
// [Low Word] [High Word]
void ModbusSlaveManager::setInt32(uint16_t start_reg, int32_t value) {
    uint16_t low_word = (uint16_t)(value & 0xFFFF);
    uint16_t high_word = (uint16_t)((value >> 16) & 0xFFFF);

    _mb.Hreg(start_reg, low_word);
    _mb.Hreg(start_reg + 1, high_word);
    
    _mb.Ireg(start_reg, low_word);
    _mb.Ireg(start_reg + 1, high_word);
}

void ModbusSlaveManager::setInt16(uint16_t start_reg, int16_t value) {
    _mb.Hreg(start_reg, (uint16_t)value);
    _mb.Ireg(start_reg, (uint16_t)value);
}

void ModbusSlaveManager::setUint16(uint16_t start_reg, uint16_t value) {
    _mb.Hreg(start_reg, value);
    _mb.Ireg(start_reg, value);
}

void ModbusSlaveManager::updateData(const MeterData& data) {
    // 0x0000 Voltage L-N (x10)
    setInt32(0x0000, (int32_t)(data.voltage * 10.0f));

    // 0x0002 Current L1 (x1000)
    setInt32(0x0002, (int32_t)(data.current * 1000.0f));

    // 0x0004 Active Power (x10) - Always Positive
    setInt32(0x0004, (int32_t)(abs(data.active_power) * 10.0f));

    // 0x0006 Apparent Power (x10)
    setInt32(0x0006, (int32_t)(data.apparent_power * 10.0f));

    // 0x0008 Reactive Power (x10)
    setInt32(0x0008, (int32_t)(data.reactive_power * 10.0f));

    // 0x000E Power Factor (x1000)
    setInt16(0x000E, (int16_t)(data.pf * 1000.0f));

    // 0x000F Frequency (x10)
    setInt16(0x000F, (int16_t)(data.frequency * 10.0f));

    // 0x0010 Total Energy (x10)
    setInt32(0x0010, (int32_t)(data.total_energy * 10.0f));
}
