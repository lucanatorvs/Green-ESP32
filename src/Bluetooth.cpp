#include "Bluetooth.h"
#include "Parameter.h"

#ifdef ENABLE_BLUETOOTH

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
        // Initialize Bluetooth controller
        if (esp_bt_controller_mem_release(ESP_BT_MODE_BLE) == ESP_OK) {
            Serial.println("Released BLE memory");
        }

        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        
        // Try to initialize with proper error handling
        if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
            Serial.println("Bluetooth controller init failed");
            return;
        }
        
        if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) != ESP_OK) {
            Serial.println("Bluetooth controller enable failed");
            esp_bt_controller_deinit();
            return;
        }
        
        if (esp_bluedroid_init() != ESP_OK) {
            Serial.println("Bluedroid init failed");
            esp_bt_controller_disable();
            esp_bt_controller_deinit();
            return;
        }
        
        if (esp_bluedroid_enable() != ESP_OK) {
            Serial.println("Bluedroid enable failed");
            esp_bluedroid_deinit();
            esp_bt_controller_disable();
            esp_bt_controller_deinit();
            return;
        }

        // Now initialize the Serial BT interface
        if (SerialBT.begin(btName)) {
            SerialBT.register_callback(onBTConnect);
            btInitialized = true;
            Serial.println("Bluetooth Serial started. Name: " + btName);
        } else {
            // Clean up if SerialBT fails
            Serial.println("Bluetooth Serial initialization failed");
            esp_bluedroid_disable();
            esp_bluedroid_deinit();
            esp_bt_controller_disable();
            esp_bt_controller_deinit();
        }
    } else {
        Serial.println("Bluetooth is already turned on");
    }
}

void turnBTOff() {
    if (btInitialized) {
        // End Bluetooth Serial
        SerialBT.end();
        
        // Additional power-saving measures
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        esp_bt_mem_release(ESP_BT_MODE_CLASSIC_BT);
        
        btInitialized = false;
        btConnected = false;
        Serial.println("Bluetooth radio completely powered off and memory released");
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

#endif // ENABLE_BLUETOOTH
