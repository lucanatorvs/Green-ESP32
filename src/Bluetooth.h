#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>

// Define to enable/disable Bluetooth functionality
// Comment out this line to disable Bluetooth
// #define ENABLE_BLUETOOTH

#ifdef ENABLE_BLUETOOTH
  #include "BluetoothSerial.h"
  #include "esp_bt.h"
  #include "esp_bt_main.h"
  #include "esp_bluedroid_api.h"

  // Initialize Bluetooth
  void initializeBluetooth();

  // Turn Bluetooth on/off
  void turnBTOn();
  void turnBTOff();

  // Check if Bluetooth is connected
  bool isBTConnected();

  // External Bluetooth Serial instance for CLI
  extern BluetoothSerial SerialBT;
#else
  // Stub functions when Bluetooth is disabled
  inline void initializeBluetooth() {}
  inline void turnBTOn() {}
  inline void turnBTOff() {}
  inline bool isBTConnected() { return false; }
#endif // ENABLE_BLUETOOTH

#endif // BLUETOOTH_H
