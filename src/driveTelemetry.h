#ifndef DRIVE_TELEMETRY_H
#define DRIVE_TELEMETRY_H

#include <Arduino.h>

// create a struct to hold the telemetry data
struct Telemetry {
    uint32_t speed;                 // (0-65535) 0 = 0 km/h, 65535 = 6553.5 km/h
    uint8_t motorTemp;              // (0-255) 0 = 40C, 255 = 295C
    uint8_t inverterTemp;           // (0-255) 0 = 40C, 255 = 295C
    int16_t rpm;                    // (0-65535) -32768 = -32768 rpm, 32767 = 32767 rpm
    float DCVoltage;                // (0-65535) 0 = 0 V, 65535 = 655.35 V
    float DCCurrent;                // (0-65535) 0 = 0 A, 65535 = 6553.5 A
    uint16_t powerUnitFlags;        // Bitmask of power unit status flags
    int16_t Current;                // (-32768-32767) -32768 = -3276.8 A, 32767 = 3276.7 A
    uint16_t Charge;                // (0-65535) 0 = 0 Ah, 65535 = 6553.5 Ah
    uint8_t SoC;                    // (0-255) 0 = 0%, 100 = 100%
    // float VoltageLimit;          // Voltage Limit
    // float CurrentLimit;          // Current Limit
    // bool ChargeContactor;        // Charge Contactor State
    // bool BatteryContactor;       // Battery Contactor State
    uint32_t Voltage;               // Voltage Least Significant Word
    // uint16_t VoltageMSW;         // Voltage Most Significant Word
    // float EstimatedEnergy;       // Estimated Energy
    // bool HighTemperatureWarning; // High Temperature Warning
    // uint8_t CustomCurrentLimit;  // Custom Current Limit
};

// powerUnitFlags bitmask:
// bit 0: SoC low for traction
// bit 1: SoC low for hydraulics
// bit 2: Reverse direction active
// bit 3: Forward direction active
// bit 4: Parking brake active
// bit 5: Pedal brake active
// bit 6: Controller is in over temperature
// bit 7: Key swich over voltage
// bit 8: Key switch under voltage
// bit 9: Vehicle is running
// bit 10: Traction is enabled
// bit 11: Hydraulics are enabled
// bit 12: Powering is enabled
// bit 13: Powering is ready

extern Telemetry telemetryData;

#endif // DRIVE_TELEMETRY_H