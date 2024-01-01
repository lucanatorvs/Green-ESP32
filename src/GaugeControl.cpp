#include "GaugeControl.h"

// Instantiate the HardwareSerial
HardwareSerial GaugeSerial(1);

// Define the ranges for each gauge
// minValue, maxValue, minAngle, maxAngle
GaugeRange SpeedometerRange     (0, 200, 17, 254);
GaugeRange TachometerRange      (0, 9000, 4, 246);
GaugeRange DynamometerRange     (-60, 90, 19, 144);
GaugeRange ChargeometerRange    (0, 100, 11, 99);
GaugeRange ThermometerRange     (-20, 100, 11, 103);

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
    Speedometer.setPosition(40);
    Tachometer.setPosition(1000);
    Dynamometer.setPosition(0);
    Chargeometer.setPosition(50);
    Thermometer.setPosition(20);
}

void sendStandbyCommand(bool enable) {
    GaugeSerial.print("STBY:");
    GaugeSerial.print(enable ? '1' : '0');
    GaugeSerial.print('\n');
}
