#ifndef DRIVE_TELEMETRY_H
#define DRIVE_TELEMETRY_H

#include <Arduino.h>

// create a struct to hold the telemetry data
struct Telemetry {
      /////////////////////////////
     // Set by PulseCounterTask //
    /////////////////////////////

    uint32_t speed;                 // (0-65535) 0 = 0 km/h, 65535 = 6553.5 km/h

      ////////////////////////////
     // Set by CANListenerTask //
    ////////////////////////////

    // CAN ID 0x06
    uint8_t motorTemp;              // (0-255) 0 = 40C, 255 = 295C
    uint8_t inverterTemp;           // (0-255) 0 = 40C, 255 = 295C
    int16_t rpm;                    // (0-65535) -32768 = -32768 rpm, 32767 = 32767 rpm
    float DCVoltage;                // (0-65535) 0 = 0 V, 65535 = 655.35 V // #todo: change this to fixed point
    float DCCurrent;                // (0-65535) 0 = 0 A, 65535 = 6553.5 A // #todo: change this to fixed point

    // CAN ID 0x07
    uint16_t powerUnitFlags;        // Bitmask of power unit status flags

    // CAN ID [EMUS] 0x99B5 0x00 0x00
    uint8_t BMSInputSignalFlags;    // Bitmask of BMS input signal flags
    uint8_t BMSOutputSignalFlags;   // Bitmask of BMS output signal flags
    uint16_t BMSNumberOfCells;      // Number of cells in the BMS detected
    uint8_t BMSChargingState;       // Charging state of the BMS
    uint16_t BMSCsDuration;         // Duration of the current charging state in minutes
    uint8_t BMSLastChargingError;   // Last charging state of the BMS

    // CAN ID [EMUS] 0x99B5 0x00 0x07
    uint32_t BMSProtectionFlags;    // Bitmask of BMS protection flags
    uint8_t BMSReductionFlags;      // Bitmask of BMS reduction flags
    uint8_t BMSBatteryStatusFlags;  // Bitmask of BMS battery status flags

    // CAN ID [EMUS] 0x99B5 0x00 0x02
    int8_t BMSMinModTemp;         // Minimum cell module temperature in degrees Celsius
    int8_t BMSMaxModTemp;         // Maximum cell module temperature in degrees Celsius
    int8_t BMSAverageModTemp;     // Average cell module temperature in degrees Celsius

    // CAN ID [EMUS] 0x99B5 0x00 0x08
    int8_t BMSMinCellTemp;        // Minimum cell temperature in degrees Celsius
    int8_t BMSMaxCellTemp;        // Maximum cell temperature in degrees Celsius
    int8_t BMSAverageCellTemp;    // Average cell temperature in degrees Celsius

    // CAN ID [EMUS] 0x05 0x00
    int16_t Current;                // (-32768-32767) -32768 = -3276.8 A, 32767 = 3276.7 A
    uint16_t Charge;                // (0-65535) 0 = 0 Ah, 65535 = 6553.5 Ah
    uint16_t SoC;                   // (0-65535) 0 = 0%, 10000 = 100% (int/100 scale)

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

// BMSInputSignalFlags bitmask:
// bit 0: Ignition Key, 1 = ignition is on
// bit 1: Charger Mains, 1 = connected
// bit 2: Fast Charge, 1 = fast charge selected
// bit 3: Leakage Sensor, 1 = leakage detected

// BMSOutputSignalFlags bitmask:
// bit 0: Charger Enable, 1 = enabled
// bit 1: Heater Enable, 1 = enabled
// bit 2: Battery Contactor, 1 = enabled
// bit 3: Battery Fan, 1 = enabled
// bit 4: Power Reduction, 1 = enabled
// bit 5: Charging Interlock, 1 = enabled
// bit 6: DCDC Control, 1 = enabled
// bit 7: Contactor Pre-Charge, 1 = enabled

// BMSChargingState values:
// 0: Disconnected – charger (any type) is disconnected
// 1: Pre-heating – battery is being pre-heated to avoid charging in low temperature
// 2: Pre-charging – battery is being pre-charged with small current
// 3: Main Charging – battery is being charged with Slow or Fast charging current (depending on Fast Charge input state)
// 4: Balancing – cells are being balanced to equalize their charge level
// 5: Charging Finished
// 6: Charging Error

// BMSLastChargingError values:
// 0: No error
// 1: No cell communication at the start of charging or communication lost during Pre-charging (using CAN charger), cannot charge
// 2: No cell communication (using Non-CAN charger), cannot charge
// 3: Maximum charging stage duration expired
// 4: Cell communication lost during Main Charging or Balancing stage (using CAN charger), cannot continue charging
// 5: Cannot set cell module balancing threshold
// 6: Cell or cell module temperature too high
// 7: Cell communication lost during Pre-heating stage (using CAN charger)
// 8: Number of cells mismatch
// 9: Cell over-voltage
// 10: Cell protection event occurred, see “Diagnostic Codes” message for determining specific protection reason

// BMSProtectionFlags bitmask:
// Bit 0: Cell Under-Voltage – one of the cell voltages is below minimum.
// Bit 1: Cell Over-Voltage – one of the cell voltages is over maximum.
// Bit 2: Discharge Over-Current – discharge current (negative current) exceeds the critical discharge current setting.
// Bit 3: Charge Over-Current – charge current (positive current) exceeds the critical charge current setting.
// Bit 4: Cell Module Over-Heat – cell module is above maximum temperature.
// Bit 5: Leakage fault – leakage signal was detected on leakage input pin.
// Bit 6: No Cell Communication – loss of communication to cells.
// Bit 7: Master-Slave Configuration Error – error with master-slave configuration.
// Bit 8: Master-Slave Internal CAN Bus Error – error with internal master-slave CAN bus.
// Bit 9: Master-Slave Common CAN Bus Error – error with common master-slave CAN bus.
// Bit 10: Charger Connected – notice that charger is connected.
// Bit 11: Cell Over-Heat – one of the cell temperatures is above maximum.
// Bit 12: No Current Sensor – no current sensor detected.
// Bit 13: Pack Under-Voltage – battery pack voltage is below minimum.
// Bit 14: Pack Over-Voltage – battery pack voltage is above maximum.
// Bit 15: Cell Under-Heat – one of the cell temperatures is below minimum.
// Bit 16: Cell Voltage Deviation – minimum and maximum cell values deviation.
// Bit 17: Pack Voltage Deviation – minimum and maximum pack voltage deviation.
// Bit 18: Cell Module Under-Heat – cell module is below minimum temperature.
// Bit 19: External Temperature Sensor Loss – loss of external temperature sensors.
// Bit 20: Wire break – cell wire break detection.
// Bit 21: String Voltage Deviation – detected difference between maximum parallel and minimum parallel voltages.
// Bit 22: Voltage And External Temperature Validation – voltages deviation between two sources.

// BMSReductionFlags bitmask:
// bit 0: Low Cell Voltage – some cells are below low voltage warning setting
// bit 1: High Discharge Current – discharge current (negative current) exceeds the current warning setting
// bit 2: High Cell Module Temperature – cell module temperature exceeds warning temperature setting
// bit 3: Master/Slave Configuration Mismatch – configuration mismatch
// bit 4: Master/Slave Common Bus Malfunction Reduction – common CAN bus malfunction
// bit 5: High Cell Temperature – cell temperature exceeds warning temperature setting

// BMSBatteryStatusFlags bitmask:
// Bit 0: Cell voltages validity (1 if valid, 0 if invalid).
// Bit 1: Cell module temperatures validity (1 if valid, 0 if invalid).
// Bit 2: Cell balancing rates validity (1 if valid, 0 if invalid).
// Bit 3: Number of live cells validity (0 if valid, 1 if invalid).
// Bit 4: Battery charging finished (1 if active, 0 if inactive). Used only with Non-CAN charger.
// Bit 5: Cell temperatures validity (1 if valid, 0 if invalid).

extern Telemetry telemetryData;

#endif // DRIVE_TELEMETRY_H