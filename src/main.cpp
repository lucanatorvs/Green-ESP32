#include <Arduino.h>
#include <nvs_flash.h>
#include <Preferences.h>

// Define parameter structure
struct Parameter {
    int index;
    String name;
    int defaultValue;
    int value;
};

// Define parameters
Parameter parameters[] = {
    // index, name, defaultValue, value
    {0, "param1", 10, 10}, // Placeholder perimeter
    {1, "param2", 20, 20}  // Placeholder perimeter
};

const int numParameters = 2; // keep synchronized with parameters array

Preferences preferences;

// Function prototypes
void cliTask(void * parameter);
void handleInput(String input);
void handleParameterCommand(String input);
void setParameter(int index, int value);
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
    xTaskCreate(cliTask, "CLI Task", 40000, NULL, 1, NULL);
}

void cliTask(void * parameter) {
    String input;
    for (;;) {
        if (Serial.available() > 0) {
            char ch = Serial.read();
            if (ch == '\b' || ch == 127) {  // ASCII for backspace or delete
                if (input.length() > 0) {
                    input.remove(input.length() - 1);  // Remove last character from input
                    Serial.write(127);  // Send DEL character
                }
            } else if (ch == '\n' || ch == '\r') {  // New line or carriage return
                Serial.println();  // Echo new line back to user immediately
                if (input.length() > 0) {  // Only process non-empty commands
                    input.trim();
                    handleInput(input);
                    input = "";  // Reset input string for next command
                }
            } else {
                input += ch;  // Accumulate characters into input string
                Serial.print(ch);  // Echo character back to user
            }
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
        for(int i = 0; i < numParameters; i++) {
            Serial.println(String(parameters[i].index) + ": " + parameters[i].name + " = " + String(parameters[i].value));
        }
    } else {
        int index = input.toInt();
        int valueIndex = input.indexOf(' ');
        if(valueIndex != -1) {
            // Set parameter value
            String valueStr = input.substring(valueIndex + 1);  // Adjusted index
            valueStr.trim();
            int value = valueStr.toInt();
            setParameter(index, value);
        } else {
            // Print parameter value
            getParameter(index);
        }
    }
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
}

void updateParametersFromNVS() {
    preferences.begin("storage", true);
    for (int i = 0; i < numParameters; i++) {
        parameters[i].value = preferences.getInt(parameters[i].name.c_str(), parameters[i].defaultValue);
    }
    preferences.end();
}

void loop() {
    // Empty loop
}