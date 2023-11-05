#include "GaugeControl.h"

// Instantiate the HardwareSerial
HardwareSerial GaugeSerial(1);

// Define the ranges for each gauge
// minValue, maxValue, minAngle, maxAngle
GaugeRange SpeedometerRange     (0, 100, 0, 270);
GaugeRange TachometerRange      (0, 100, 0, 270);
GaugeRange DynamometerRange     (0, 100, 0, 270);
GaugeRange ChargeometerRange    (0, 100, 0, 270);
GaugeRange ThermometerRange     (0, 100, 0, 270);

// Instantiate each gauge
Gauge Speedometer   ("Speedometer",     GaugeSerial, SpeedometerRange);
Gauge Tachometer    ("Tachometer",      GaugeSerial, TachometerRange);
Gauge Dynamometer   ("Dynamometer",     GaugeSerial, DynamometerRange);
Gauge Chargeometer  ("Chargeometer",    GaugeSerial, ChargeometerRange);
Gauge Thermometer   ("Thermometer",     GaugeSerial, ThermometerRange);

void initializeGaugeControl() {
    // Initialize the serial communication for gauges
    GaugeSerial.begin(9600, SERIAL_8N1, GaugeRX, GaugeTX);

    // enable standby mode
    sendStandbyCommand(true);
    
    // set all gauges to 0 position
    Speedometer.setPosition(0);
    Tachometer.setPosition(0);
    Dynamometer.setPosition(0);
    Chargeometer.setPosition(0);
    Thermometer.setPosition(0);
}

void sendStandbyCommand(bool enable) {
    GaugeSerial.print("STBY:");
    GaugeSerial.print(enable ? '1' : '0');
    GaugeSerial.print('\n');
}
