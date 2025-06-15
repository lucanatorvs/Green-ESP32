#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include "BluetoothSerial.h"

// Initialize OTA updates
void initializeOTA();

// Start/restart WiFi connection
bool startWiFi();

// Get WiFi status as a string
String getWiFiStatusString();

// Enable or disable WiFi
bool setWiFiEnabled(bool enabled);

// Check if WiFi is enabled
bool isWiFiEnabled();

// Check if Bluetooth is connected
bool isBTConnected();

// Set WiFi credentials
bool setWiFiCredentials(const String &ssid, const String &password);

// Get WiFi SSID
String getWiFiSSID();

// External Bluetooth Serial instance for CLI
extern BluetoothSerial SerialBT;

#endif // OTA_H