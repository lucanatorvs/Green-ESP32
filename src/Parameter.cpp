#include "Parameter.h"
#include <nvs_flash.h>
#include <Preferences.h>

Preferences preferences;

Parameter parameters[] = {
    // index, name, defaultValue, value     // unit
    {0, "OdometerCount", 199000, 199000},   // Kilometers
    {1, "BlinkSpeed", 500, 500},            // Milliseconds
    {2, "PulseDelay", 100, 100},            // Milliseconds
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

void setParameter(int index, int value) {
    if (index >= 0 && index < numParameters) {
        parameters[index].value = value;
        Serial.println("Set " + parameters[index].name + " to " + String(value));
        storeParametersToNVS();
    } else {
        Serial.println("Invalid index");
    }
}

void getParameter(int index) {
    if (index >= 0 && index < numParameters) {
        Serial.println(String(index) + ": " + parameters[index].name + " = " + String(parameters[index].value));
    } else {
        Serial.println("Invalid index");
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
    }
    preferences.end();
}

void clearNVS(int index) {
    preferences.begin("storage", false);
    if (index == -1) {
        // Clear all parameters
        preferences.clear();
        for(int i = 0; i < numParameters; i++) {
            parameters[i].value = parameters[i].defaultValue;
        }
        Serial.println("NVS cleared, default values restored");
    } else if (index >= 0 && index < numParameters) {
        // Clear specified parameter
        preferences.remove(parameters[index].name.c_str());
        parameters[index].value = parameters[index].defaultValue;
        Serial.println("Parameter " + String(index) + " (" + parameters[index].name + ") cleared, default value restored");
    } else {
        Serial.println("Error: Invalid index");
    }
    preferences.end();
    storeParametersToNVS();
}

void updateParametersFromNVS(int index) {
    preferences.begin("storage", true);
    if (index == -1) {
        // Update all parameters
        for (int i = 0; i < numParameters; i++) {
            parameters[i].value = preferences.getInt(parameters[i].name.c_str(), parameters[i].defaultValue);
        }
        Serial.println("Parameters updated from NVS");
    } else if (index >= 0 && index < numParameters) {
        // Update specified parameter
        parameters[index].value = preferences.getInt(parameters[index].name.c_str(), parameters[index].defaultValue);
        Serial.println("Parameter " + String(index) + " (" + parameters[index].name + ") updated from NVS");
    } else {
        Serial.println("Error: Invalid index");
    }
    preferences.end();
}
