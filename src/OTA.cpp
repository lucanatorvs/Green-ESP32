#include "OTA.h"
#include "Parameter.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "BluetoothSerial.h"
#include <Preferences.h>

// Preferences namespace for WiFi settings
#define WIFI_NAMESPACE "wifi"
#define WIFI_SSID_KEY "ssid"
#define WIFI_PASS_KEY "password"
#define WIFI_ENABLED_KEY "enabled"

// Default OTA password
const char* DEFAULT_PASSWORD = "Electric3z";

// WiFi connection status
bool wifiConnected = false;
bool wifiEnabled = false;
TaskHandle_t wifiTaskHandle = NULL;

// Bluetooth Serial instance
BluetoothSerial SerialBT;
bool btConnected = false;

// Forward declarations
void wifiTask(void * parameter);
void onBTConnect(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

void initializeOTA() {
    // Initialize Bluetooth Serial - simple version with no PIN
    String btName = "Green-ESP32";
    SerialBT.begin(btName);
    SerialBT.register_callback(onBTConnect);
    
    Serial.println("Bluetooth Serial started. Name: " + btName);
    
    // Load WiFi enabled state from preferences
    Preferences preferences;
    preferences.begin(WIFI_NAMESPACE, true);
    wifiEnabled = preferences.getBool(WIFI_ENABLED_KEY, false);  // Default to disabled for safety
    preferences.end();
    
    // Initialize WiFi in STA mode at startup (don't change mode later)
    // This single initialization avoids the deinit errors
    WiFi.mode(WIFI_STA);
    
    // Start WiFi task if enabled
    if (wifiEnabled) {
        xTaskCreate(wifiTask, "WiFi Task", 4096, NULL, 1, &wifiTaskHandle);
    }
}

void setupOTA() {
    // Get hostname from preferences or use default
    String hostname = "Green-ESP32";
    
    // Set hostname for OTA
    ArduinoOTA.setHostname(hostname.c_str());
    
    // Set password for OTA
    ArduinoOTA.setPassword(DEFAULT_PASSWORD);
    
    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH) {
                type = "sketch";
            } else {
                type = "filesystem";
            }
            Serial.println("Start updating " + type);
            if (isBTConnected()) {
                SerialBT.println("Start updating " + type);
            }
        })
        .onEnd([]() {
            Serial.println("\nEnd");
            if (isBTConnected()) {
                SerialBT.println("\nEnd");
            }
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            unsigned int percentComplete = (progress / (total / 100));
            Serial.printf("Progress: %u%%\r", percentComplete);
            if (isBTConnected()) {
                SerialBT.printf("Progress: %u%%\r", percentComplete);
            }
        })
        .onError([](ota_error_t error) {
            String errorMsg;
            if (error == OTA_AUTH_ERROR) errorMsg = "Auth Failed";
            else if (error == OTA_BEGIN_ERROR) errorMsg = "Begin Failed";
            else if (error == OTA_CONNECT_ERROR) errorMsg = "Connect Failed";
            else if (error == OTA_RECEIVE_ERROR) errorMsg = "Receive Failed";
            else if (error == OTA_END_ERROR) errorMsg = "End Failed";
            
            Serial.printf("Error[%u]: %s\n", error, errorMsg.c_str());
            if (isBTConnected()) {
                SerialBT.printf("Error[%u]: %s\n", error, errorMsg.c_str());
            }
        });

    ArduinoOTA.begin();
    Serial.println("OTA initialized");
    if (isBTConnected()) {
        SerialBT.println("OTA initialized");
    }
}

bool startWiFi() {
    // First check if WiFi is enabled
    if (!wifiEnabled) {
        Serial.println("WiFi is disabled. Use 'wifi on' to enable it.");
        if (isBTConnected()) {
            SerialBT.println("WiFi is disabled. Use 'wifi on' to enable it.");
        }
        return false;
    }
    
    // Get SSID and password from preferences
    Preferences preferences;
    preferences.begin(WIFI_NAMESPACE, true); // Read-only mode
    String ssid = preferences.getString(WIFI_SSID_KEY, "");
    String password = preferences.getString(WIFI_PASS_KEY, "");
    preferences.end();
    
    if (ssid.length() == 0) {
        Serial.println("WiFi SSID not set. Use 'wifi ssid <name>' to set it.");
        if (isBTConnected()) {
            SerialBT.println("WiFi SSID not set. Use 'wifi ssid <name>' to set it.");
        }
        return false;
    }

    // Just disconnect - don't try to change mode or deinitialize
    WiFi.disconnect(false, false);  // disconnect but don't turn off WiFi
    delay(500);
    
    // Now begin connection with saved credentials
    WiFi.begin(ssid.c_str(), password.c_str());
    
    Serial.println("Connecting to WiFi...");
    Serial.print("SSID: ");
    Serial.println(ssid);
    
    if (isBTConnected()) {
        SerialBT.println("Connecting to WiFi...");
        SerialBT.print("SSID: ");
        SerialBT.println(ssid);
    }
    
    // Wait for connection with timeout
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(500);
        Serial.print(".");
        if (isBTConnected()) {
            SerialBT.print(".");
        }
        retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        if (isBTConnected()) {
            SerialBT.println("");
            SerialBT.print("Connected to ");
            SerialBT.println(ssid);
            SerialBT.print("IP address: ");
            SerialBT.println(WiFi.localIP().toString());
        }
        
        // Set up OTA updates
        setupOTA();
        
        wifiConnected = true;
        return true;
    } else {
        Serial.println("");
        Serial.println("Connection failed!");
        
        if (isBTConnected()) {
            SerialBT.println("");
            SerialBT.println("Connection failed!");
        }
        
        wifiConnected = false;
        return false;
    }
}

void wifiTask(void * parameter) {
    // Try to start WiFi
    startWiFi();
    
    // Keep monitoring WiFi status
    for (;;) {
        // Check if OTA updates need to be handled
        if (wifiConnected) {
            ArduinoOTA.handle();
        }
        
        // If WiFi was connected but is now disconnected, try to reconnect
        if (wifiEnabled && wifiConnected && WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi connection lost. Attempting to reconnect...");
            if (isBTConnected()) {
                SerialBT.println("WiFi connection lost. Attempting to reconnect...");
            }
            
            // Wait a bit before trying to reconnect
            vTaskDelay(pdMS_TO_TICKS(5000));
            
            // Try to reconnect
            WiFi.disconnect();
            delay(500);
            startWiFi();
        }
        
        // Short delay to prevent hogging the CPU
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void onBTConnect(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (event == ESP_SPP_SRV_OPEN_EVT) {
        Serial.println("Bluetooth client connected");
        btConnected = true;
        
        // Send welcome message
        SerialBT.println("\nWelcome to Green-ESP32 CLI");
        SerialBT.println("Type 'help' for available commands");
        
        // Send WiFi status if enabled
        if (wifiEnabled) {
            SerialBT.print("WiFi Status: ");
            SerialBT.println(getWiFiStatusString());
            if (wifiConnected) {
                SerialBT.print("IP Address: ");
                SerialBT.println(WiFi.localIP().toString());
            }
        }
    } else if (event == ESP_SPP_CLOSE_EVT) {
        Serial.println("Bluetooth client disconnected");
        btConnected = false;
    }
}

bool isBTConnected() {
    return btConnected;
}

String getWiFiStatusString() {
    if (!wifiEnabled) {
        return "Disabled";
    }
    
    switch (WiFi.status()) {
        case WL_CONNECTED:     return "Connected";
        case WL_IDLE_STATUS:   return "Idle";
        case WL_DISCONNECTED:  return "Disconnected";
        case WL_CONNECT_FAILED: return "Connection Failed";
        case WL_NO_SHIELD:     return "No WiFi shield";
        case WL_NO_SSID_AVAIL: return "SSID not available";
        case WL_SCAN_COMPLETED: return "Scan completed";
        default:               return "Unknown status";
    }
}

bool setWiFiEnabled(bool enabled) {
    if (wifiEnabled == enabled) {
        return true; // Already in the requested state
    }
    
    wifiEnabled = enabled;
    
    // Store the setting in preferences
    Preferences preferences;
    preferences.begin(WIFI_NAMESPACE, false);
    preferences.putBool(WIFI_ENABLED_KEY, enabled);
    preferences.end();
    
    if (enabled) {
        // Start WiFi task if it's not already running
        if (wifiTaskHandle == NULL) {
            xTaskCreate(wifiTask, "WiFi Task", 4096, NULL, 1, &wifiTaskHandle);
            return true;
        } else {
            return startWiFi();
        }
    } else {
        // Disable WiFi by disconnecting only - don't change mode
        wifiConnected = false;
        WiFi.disconnect(false, false);  // disconnect but don't turn off WiFi
        
        // Delete the task if it exists
        if (wifiTaskHandle != NULL) {
            vTaskDelete(wifiTaskHandle);
            wifiTaskHandle = NULL;
        }
        
        return true;
    }
}

bool isWiFiEnabled() {
    return wifiEnabled;
}

bool setWiFiCredentials(const String &ssid, const String &password) {
    if (ssid.length() == 0) {
        return false;
    }
    
    Preferences preferences;
    preferences.begin(WIFI_NAMESPACE, false);
    preferences.putString(WIFI_SSID_KEY, ssid);
    preferences.putString(WIFI_PASS_KEY, password);
    preferences.end();
    
    return true;
}

String getWiFiSSID() {
    Preferences preferences;
    preferences.begin(WIFI_NAMESPACE, true);
    String ssid = preferences.getString(WIFI_SSID_KEY, "");
    preferences.end();
    
    return ssid;
}