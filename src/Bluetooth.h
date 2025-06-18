#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include "BluetoothSerial.h"

// Initialize Bluetooth
void initializeBluetooth();

// Turn Bluetooth on/off
void turnBTOn();
void turnBTOff();

// Check if Bluetooth is connected
bool isBTConnected();

// External Bluetooth Serial instance for CLI
extern BluetoothSerial SerialBT;

#endif // BLUETOOTH_H
