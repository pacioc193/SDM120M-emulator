// ModbusManager.cpp
#include "ModbusManager.h"
#include "Logger.h"
#include <Arduino.h>

void ModbusManager::begin(DataManager& client, uint8_t slaveId, int rxPin, int txPin, int deRePin, int iBoud) {
	pxClient = &client;

	Serial2.begin(iBoud, SERIAL_8N1, rxPin, txPin); // Typical Modbus RTU settings
	mb.setBaudrate(iBoud); // Set baud rate separately for ModbusRTU object
	mb.slave(slaveId);
	mb.begin(&Serial2, deRePin);

	// Initialize lastGoodMillis to now so transient startup doesn't show offline immediately
	lastGoodMillis = millis();

	// Add holding registers to the Modbus map. Each float takes 2 registers.
	mb.addIreg(SDM120_REG_VOLTAGE, 0, 2);
	mb.addIreg(SDM120_REG_CURRENT, 0, 2);
	mb.addIreg(SDM120_REG_ACTIVE_POWER, 0, 2);
	mb.addIreg(SDM120_REG_APPARENT_POWER, 0, 2);
	mb.addIreg(SDM120_REG_REACTIVE_POWER, 0, 2);
	mb.addIreg(SDM120_REG_POWER_FACTOR, 0, 2);
	mb.addIreg(SDM120_REG_PHASE_ANGLE, 0, 2);
	mb.addIreg(SDM120_REG_FREQUENCY, 0, 2);
	mb.addIreg(SDM120_REG_TOTAL_ACTIVE_ENERGY, 0, 2);


    // Pass haClient (pHaClient) to the lambda by capturing it explicitly
    mb.onRequest([this](Modbus::FunctionCode fc, Modbus::RequestData data) -> Modbus::ResultCode {

        if (fc != Modbus::FC_READ_INPUT_REGS) {
            Logger::getInstance().log(LOG_TAG_MODBUS, "Unsupported Modbus function code: " + String(fc) + ", Address: " + String(data.reg.address) + ", Count: " + String(data.regCount));
            return Modbus::EX_ILLEGAL_FUNCTION;
        }
        else
        {
            // Consider client online if it reported data recently within timeout
            unsigned long now = millis();
            bool recentGood = (now - lastGoodMillis <= MODBUS_OFFLINE_TIMEOUT_MS);
            if (!recentGood) {
                Logger::getInstance().log(LOG_TAG_MODBUS, "Client considered offline (no recent data). Cannot process Modbus request.");
                return Modbus::EX_SLAVE_FAILURE;
            }
            Logger::getInstance().log(LOG_TAG_MODBUS, "Received Modbus request: Function Code: " + String(fc) + ", Address: " + String(data.reg.address) + ", Count: " + String(data.regCount));
        }
        return Modbus::EX_SUCCESS;
    });
	
	Logger::getInstance().log(LOG_TAG_MODBUS, "Modbus RTU Slave initialized on Serial2.");
	Logger::getInstance().log(LOG_TAG_MODBUS, "Modbus Slave ID: " + String(slaveId));
	Logger::getInstance().log(LOG_TAG_MODBUS, "Baud rate: "+String(iBoud)+", Format: 8N1");
}

void ModbusManager::modbus_loop() {
	// Process Modbus requests
	mb.task();
	// Update register values from Home Assistant data
	updateModbusRegisters();
}

void ModbusManager::updateModbusRegisters() {
	// Get the latest data from Home Assistant client
	MeterData data = pxClient->getCurrentData();

	// If the client reports online, update lastGoodMillis
	if (pxClient->isOnline()) {
		lastGoodMillis = millis();
	}

	// Convert float values to two uint16_t and write to Modbus registers
	// Standard Modbus float representation is usually Big-Endian (MSW at lower address)
	float_to_uint16 conv;

	conv.f = data.voltage_v;
	mb.Ireg(SDM120_REG_VOLTAGE, conv.i[1]);
	mb.Ireg(SDM120_REG_VOLTAGE + 1, conv.i[0]);

	conv.f = data.current_a;
	mb.Ireg(SDM120_REG_CURRENT, conv.i[1]);
	mb.Ireg(SDM120_REG_CURRENT + 1, conv.i[0]);

	conv.f = data.power_w;
	mb.Ireg(SDM120_REG_ACTIVE_POWER, conv.i[1]);
	mb.Ireg(SDM120_REG_ACTIVE_POWER + 1, conv.i[0]);

	float apparent_power = (data.voltage_v * data.current_a); // in kVA
	conv.f = apparent_power;
	mb.Ireg(SDM120_REG_APPARENT_POWER, conv.i[1]);
	mb.Ireg(SDM120_REG_APPARENT_POWER + 1, conv.i[0]);

	conv.f = apparent_power * sqrt(1.0 - data.power_factor * data.power_factor); // in kVAR
	mb.Ireg(SDM120_REG_REACTIVE_POWER, conv.i[1]);
	mb.Ireg(SDM120_REG_REACTIVE_POWER + 1, conv.i[0]);

	conv.f = data.power_factor;
	mb.Ireg(SDM120_REG_POWER_FACTOR, conv.i[1]);
	mb.Ireg(SDM120_REG_POWER_FACTOR + 1, conv.i[0]);

	conv.f = data.frequency_hz;
	mb.Ireg(SDM120_REG_FREQUENCY, conv.i[1]);
	mb.Ireg(SDM120_REG_FREQUENCY + 1, conv.i[0]);
	
	conv.f = acos(data.power_factor) * (180.0 / M_PI); // Convert to degrees
	mb.Ireg(SDM120_REG_PHASE_ANGLE, conv.i[1]);
	mb.Ireg(SDM120_REG_PHASE_ANGLE + 1, conv.i[0]);

	conv.f = data.energy_kwh;
	mb.Ireg(SDM120_REG_TOTAL_ACTIVE_ENERGY, conv.i[1]);
	mb.Ireg(SDM120_REG_TOTAL_ACTIVE_ENERGY + 1, conv.i[0]);
}