#include "Parameter.h"
#include <nvs_flash.h>
#include <Preferences.h>

Preferences preferences;

Parameter parameters[] = {
    {0, "param1", 10, 10},
    {1, "param2", 20, 20}
};

const int numParameters = 2;

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

void storeParametersToNVS() {
    preferences.begin("storage", false);
    for (int i = 0; i < numParameters; i++) {
        preferences.putInt(parameters[i].name.c_str(), parameters[i].value);
    }
    preferences.end();
}

void clearNVS() {
    preferences.begin("storage", false);
    preferences.clear();
    preferences.end();
    for(int i = 0; i < numParameters; i++) {
        parameters[i].value = parameters[i].defaultValue;
    }
    storeParametersToNVS();
    Serial.println("NVS cleared, default values restored");
}

void updateParametersFromNVS() {
    preferences.begin("storage", true);
    for (int i = 0; i < numParameters; i++) {
        parameters[i].value = preferences.getInt(parameters[i].name.c_str(), parameters[i].defaultValue);
    }
    preferences.end();
    Serial.println("Parameters updated from NVS");
}
