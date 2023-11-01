#include <Arduino.h>
#include <nvs_flash.h>
#include <Preferences.h>

// Define default parameters
struct Parameters {
    int param1 = 10;
    float param2 = 20.5;
};

Parameters params;
Preferences preferences;

// Function prototypes
void cliTask(void * parameter);
void handleInput(String input);
void handleParameterCommand(String input);
void setParameter(int index, float value);
void getParameter(int index);
void storeParametersToNVS();
void clearNVS();
void updateParametersFromNVS();

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
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
    // Create CLI task
    xTaskCreate(cliTask, "CLI Task", 10000, NULL, 1, NULL);
}

void cliTask(void * parameter) {
    for (;;) {
        if(Serial.available() > 0) {
            String input = Serial.readStringUntil('\n');
            input.trim();  // Remove any trailing whitespace
            handleInput(input);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void handleInput(String input) {
    if(input.startsWith("echo ")) {
        Serial.println(input.substring(5));
    } else if(input.startsWith("p")) {
        String paramInput = input.substring(1);
        paramInput.trim();
        handleParameterCommand(paramInput);
    } else if(input == "clear nvs") {
        clearNVS();
    } else if(input == "update nvs") {
        updateParametersFromNVS();
    } else {
        Serial.println("Unknown command");
    }
}

void handleParameterCommand(String input) {
    if(input == "" || input == "h" || input == "help") {
        // List all parameters
        Serial.println("0: param1 = " + String(params.param1));
        Serial.println("1: param2 = " + String(params.param2));
    } else {
        int index = input.toInt();
        int valueIndex = input.indexOf(' ');
        if(valueIndex != -1) {
            // Set parameter value
            String valueStr = input.substring(valueIndex + 1);  // Adjusted index
            valueStr.trim();
            float value = valueStr.toFloat();
            setParameter(index, value);
        } else {
            // Print parameter value
            getParameter(index);
        }
    }
}

void setParameter(int index, float value) {
    if(index == 0) {
        params.param1 = (int)value;
    } else if(index == 1) {
        params.param2 = value;
    }
    storeParametersToNVS();
}

void getParameter(int index) {
    if(index == 0) {
        Serial.println("0: param1 = " + String(params.param1));
    } else if(index == 1) {
        Serial.println("1: param2 = " + String(params.param2));
    } else {
        Serial.println("Invalid index");
    }
}

void storeParametersToNVS() {
    preferences.begin("storage", false);
    preferences.putInt("param1", params.param1);
    preferences.putFloat("param2", params.param2);
    preferences.end();
}

void clearNVS() {
    preferences.begin("storage", false);
    preferences.clear();
    preferences.end();
    params.param1 = 10;
    params.param2 = 20.5;
    storeParametersToNVS();
}

void updateParametersFromNVS() {
    preferences.begin("storage", true);
    params.param1 = preferences.getInt("param1", 10);
    params.param2 = preferences.getFloat("param2", 20.5);
    preferences.end();
}

void loop() {
    // Empty loop
}