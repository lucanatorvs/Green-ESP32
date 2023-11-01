#include "CLI.h"
#include "Parameter.h"

void initializeCLI() {
    // Initialize Serial communication
    Serial.begin(115200);
    // Create CLI task
    xTaskCreate(cliTask, "CLI Task", 2048, NULL, 2, NULL);
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
        vTaskDelay(10 / portTICK_PERIOD_MS); // Wait 10ms
    }
}

void handleInput(String input) {
    if(input.startsWith("echo ")) {
        Serial.println(input.substring(5));
    } else if(input.startsWith("p")) {
        String paramInput = input.substring(1);
        paramInput.trim();
        handleParameterCommand(paramInput);
    } else if(input == "clear") {
        clearNVS();
    } else if(input == "update") {
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
