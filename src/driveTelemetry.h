#ifndef DRIVE_TELEMETRY_H
#define DRIVE_TELEMETRY_H

#include <Arduino.h>

// create a struct to hold the telemetry data
struct Telemetry {
    uint8_t motorTemp;          // (0-255) 0 = 40C, 255 = 295C
    uint8_t inverterTemp;       // (0-255) 0 = 40C, 255 = 295C
    uint16_t rpm;               // (0-65535) 0 = 0 rpm, 65535 = 65535 rpm
    float DCVoltage;         // (0-65535) 0 = 0 V, 65535 = 655.35 V
    float DCCurrent;         // (0-65535) 0 = 0 A, 65535 = 6553.5 A
    uint16_t powerUnitFlags;    // Bitmask of power unit status flags
};

extern Telemetry telemetryData;

#endif // DRIVE_TELEMETRY_H