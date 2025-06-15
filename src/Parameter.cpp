#include "Parameter.h"
#include <nvs_flash.h>
#include <Preferences.h>
#include "BluetoothSerial.h"

// Forward declare functions and variables from OTA
extern bool isBTConnected();
extern BluetoothSerial SerialBT;

Preferences preferences;

Parameter parameters[] = {
    // index, name, defaultValue, value     // unit
    {0, "OdometerCount", 202600, 202600},   // Kilometers
    {1, "BlinkSpeed", 500, 500},            // Milliseconds
    {2, "PulseDelay", 100, 100},            // Milliseconds for the pulse counter to integrate pulses
    {3, "SpeedFactor", 800, 800}            // mm per pulse
};

const int numParameters = sizeof(parameters) / sizeof(parameters[0]);

void initializeParameter() {
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    // Load parameters from NVS
    updateParametersFromNVS();
}

void setParameter(int index, int value, Stream *output) {
    if (index >= 0 && index < numParameters) {
        parameters[index].value = value;
        
        // Output to specified stream if provided
        if (output) {
            output->println("Set " + parameters[index].name + " to " + String(value));
        } else {
            Serial.println("Set " + parameters[index].name + " to " + String(value));
            // Also output to Bluetooth if connected
            if (isBTConnected()) {
                SerialBT.println("Set " + parameters[index].name + " to " + String(value));
            }
        }
        
        storeParametersToNVS();
    } else {
        if (output) {
            output->println("Invalid index");
        } else {
            Serial.println("Invalid index");
            if (isBTConnected()) {
                SerialBT.println("Invalid index");
            }
        }
    }
}

void getParameter(int index, Stream *output) {
    if (index >= 0 && index < numParameters) {
        if (output) {
            output->println(String(index) + ": " + parameters[index].name + " = " + String(parameters[index].value));
        } else {
            Serial.println(String(index) + ": " + parameters[index].name + " = " + String(parameters[index].value));
            if (isBTConnected()) {
                SerialBT.println(String(index) + ": " + parameters[index].name + " = " + String(parameters[index].value));
            }
        }
    } else {
        if (output) {
            output->println("Invalid index");
        } else {
            Serial.println("Invalid index");
            if (isBTConnected()) {
                SerialBT.println("Invalid index");
            }
        }
    }
}

void storeParametersToNVS(int index) {
    preferences.begin("storage", false);
    if (index == -1) {
        // Default case: save all parameters
        for (int i = 0; i < numParameters; i++) {
            preferences.putInt(parameters[i].name.c_str(), parameters[i].value);
        }
    } else if (index >= 0 && index < numParameters) {
        // Specific parameter case: save specified parameter
        preferences.putInt(parameters[index].name.c_str(), parameters[index].value);
    } else {
        // Error case: invalid index
        Serial.println("Error: Invalid index");
        if (isBTConnected()) {
            SerialBT.println("Error: Invalid index");
        }
    }
    preferences.end();
}

void clearNVS(int index, Stream *output) {
    preferences.begin("storage", false);
    if (index == -1) {
        // Clear all parameters
        preferences.clear();
        for(int i = 0; i < numParameters; i++) {
            parameters[i].value = parameters[i].defaultValue;
        }
        
        if (output) {
            output->println("NVS cleared, default values restored");
        } else {
            Serial.println("NVS cleared, default values restored");
            if (isBTConnected()) {
                SerialBT.println("NVS cleared, default values restored");
            }
        }
    } else if (index >= 0 && index < numParameters) {
        // Clear specified parameter
        preferences.remove(parameters[index].name.c_str());
        parameters[index].value = parameters[index].defaultValue;
        
        if (output) {
            output->println("Parameter " + String(index) + " (" + parameters[index].name + ") cleared, default value (" + String(parameters[index].defaultValue) + ") restored");
        } else {
            Serial.println("Parameter " + String(index) + " (" + parameters[index].name + ") cleared, default value (" + String(parameters[index].defaultValue) + ") restored");
            if (isBTConnected()) {
                SerialBT.println("Parameter " + String(index) + " (" + parameters[index].name + ") cleared, default value (" + String(parameters[index].defaultValue) + ") restored");
            }
        }
    } else {
        if (output) {
            output->println("Error: Invalid index");
        } else {
            Serial.println("Error: Invalid index");
            if (isBTConnected()) {
                SerialBT.println("Error: Invalid index");
            }
        }
    }
    preferences.end();
    storeParametersToNVS();
}

void updateParametersFromNVS(int index, Stream *output) {
    preferences.begin("storage", true);
    if (index == -1) {
        // Update all parameters
        for (int i = 0; i < numParameters; i++) {
            parameters[i].value = preferences.getInt(parameters[i].name.c_str(), parameters[i].defaultValue);
        }
        
        if (output) {
            output->println("Parameters updated from NVS");
        } else {
            Serial.println("Parameters updated from NVS");
            if (isBTConnected()) {
                SerialBT.println("Parameters updated from NVS");
            }
        }
    } else if (index >= 0 && index < numParameters) {
        // Update specified parameter
        parameters[index].value = preferences.getInt(parameters[index].name.c_str(), parameters[index].defaultValue);
        
        if (output) {
            output->println("Parameter " + String(index) + " (" + parameters[index].name + ") updated from NVS");
        } else {
            Serial.println("Parameter " + String(index) + " (" + parameters[index].name + ") updated from NVS");
            if (isBTConnected()) {
                SerialBT.println("Parameter " + String(index) + " (" + parameters[index].name + ") updated from NVS");
            }
        }
    } else {
        if (output) {
            output->println("Error: Invalid index");
        } else {
            Serial.println("Error: Invalid index");
            if (isBTConnected()) {
                SerialBT.println("Error: Invalid index");
            }
        }
    }
    preferences.end();
}
