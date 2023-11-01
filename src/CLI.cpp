#include "CLI.h"
#include "Parameter.h"
#include "PulseCounterTask.h"

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
    } else if(input == "s" || input == "speed") {
        handleSpeedCommand();
    } else if(input == "h" || input == "help") {
        // Print help message for all commands
        Serial.println("Available commands:");
        Serial.println("  echo [text]       - Echoes the text back to the serial output.");
        Serial.println("  p [subcommand]    - Parameter command. Type 'p h' or 'p help' for more information.");
        Serial.println("  s, speed          - Prints the current speed value.");
        Serial.println("  h, help           - Displays this help message.");
    } else {
        Serial.println("Unknown command");
    }
}

void handleSpeedCommand() {
    uint32_t currentSpeed = getSpeed();  // Assuming getSpeed() is accessible
    Serial.println("Current Speed: " + String(currentSpeed) + " Hz");  // Adjust units if necessary
}

void handleParameterCommand(String input) {
    if(input == "h" || input == "help") {
        // Print help message
        Serial.println("Usage: p [index] [value] | p update [index] | p clear [index]");
        Serial.println("  index: parameter index");
        Serial.println("  value: new value for parameter");
        Serial.println("  p update: update parameters from NVS");
        Serial.println("  p clear: clear NVS and reset parameters to default");
    } else if(input.startsWith("clear")) {
        String indexStr = input.substring(6);
        indexStr.trim();
        if (indexStr.length() > 0) {
            if (!indexStr.toInt() && indexStr != "0") {
                Serial.println("Error: Invalid index");
                return;
            }
            clearNVS(indexStr.toInt());
        } else {
            clearNVS();
        }
    } else if(input.startsWith("update")) {
        String indexStr = input.substring(7);
        indexStr.trim();
        if (indexStr.length() > 0) {
            if (!indexStr.toInt() && indexStr != "0") {
                Serial.println("Error: Invalid index");
                return;
            }
            updateParametersFromNVS(indexStr.toInt());
        } else {
            updateParametersFromNVS();
        }
    } else if(input == "") {
        // List all parameters
        for(int i = 0; i < numParameters; i++) {
            Serial.println(String(parameters[i].index) + ": " + parameters[i].name + " = " + String(parameters[i].value));
        }
    } else {
        int valueIndex = input.indexOf(' ');
        if(valueIndex != -1) {
            String indexStr = input.substring(0, valueIndex);
            String valueStr = input.substring(valueIndex + 1);
            indexStr.trim();
            valueStr.trim();
            // Check for invalid input
            if (!indexStr.toInt() && indexStr != "0") {
                Serial.println("Error: Invalid index");
                return;
            }
            if (!valueStr.toInt() && valueStr != "0") {
                Serial.println("Error: Invalid value");
                return;
            }
            int index = indexStr.toInt();
            int value = valueStr.toInt();
            setParameter(index, value);
        } else {
            // Check for invalid index
            if (!input.toInt() && input != "0") {
                Serial.println("Error: Invalid index");
                return;
            }
            int index = input.toInt();
            getParameter(index);
        }
    }
}
