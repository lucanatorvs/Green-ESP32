#include "Bluetooth.h"
#include "Parameter.h"

// Bluetooth Serial instance
BluetoothSerial SerialBT;
bool btConnected = false;
bool btInitialized = false;
String btName = "Green-ESP32";

// Forward declarations
void onBTConnect(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

void initializeBluetooth() {
    // Initialize Bluetooth Serial - simple version with no PIN
    turnBTOn();
}

void turnBTOn() {
    if (!btInitialized) {
        SerialBT.begin(btName);
        SerialBT.register_callback(onBTConnect);
        btInitialized = true;
        Serial.println("Bluetooth Serial started. Name: " + btName);
    } else {
        Serial.println("Bluetooth is already turned on");
    }
}

void turnBTOff() {
    if (btInitialized) {
        SerialBT.end();
        btInitialized = false;
        btConnected = false;
        Serial.println("Bluetooth Serial stopped");
    } else {
        Serial.println("Bluetooth is already turned off");
    }
}

void onBTConnect(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (event == ESP_SPP_SRV_OPEN_EVT) {
        Serial.println("Bluetooth client connected");
        btConnected = true;
        
        // Send welcome message
        SerialBT.println("\nWelcome to Green-ESP32 CLI");
        SerialBT.println("Type 'help' for available commands");
    } else if (event == ESP_SPP_CLOSE_EVT) {
        Serial.println("Bluetooth client disconnected");
        btConnected = false;
    }
}

bool isBTConnected() {
    return btConnected;
}
