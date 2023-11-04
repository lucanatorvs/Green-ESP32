#include "CLI.h"
#include "Parameter.h"
#include "PulseCounterTask.h"

// Predefine commands as constants for consistency and easy modification
const String CMD_HELP = "help";
const String CMD_ECHO = "echo ";
const String CMD_PARAM = "p";
const String CMD_SPEED = "s";
const String CMD_SYS = "sys";
const String CMD_TRIP = "trip";
const String CMD_RESET = "reset";

// Command descriptions
const String HELP_TEXT = "Available commands:\n"
                         "  echo [text]         - Echoes the text back to the serial output.\n"
                         "  help                - Displays this help message.\n"
                         "  info                - Displays system information.\n"
                         "  p [subcommand]      - Parameter command. Type 'p help' for more information.\n"
                         "  reset               - Resets the ESP32.\n"
                         "  s                   - Prints the current speed measurement.\n"
                         "  sys                 - Displays system information.\n"
                         "  trip [subcommand]   - Trip odometer command. Type 'trip help' for more information.\n";

const String PARAM_HELP_TEXT = "Usage: p [index] [value] | p update [index] | p clear [index]\n"
                               "  index: parameter index\n"
                               "  value: new value for parameter\n"
                               "  p update: update parameters from NVS\n"
                               "  p clear: clear NVS and reset parameters to default\n";

const String TRIP_HELP_TEXT = "Usage: trip [subcommand]\n"
                              "  subcommand: r, reset\n"
                              "  subcommand: s, set [value] km\n"
                              "  subcommand: help\n";

void cliTask(void * parameter);
void handleInput(String input);
void handleParameterCommand(String input);
void handleSpeedCommand();
void handleInfoCommand();
void handleTripCommand(String input);

void initializeCLI() {
    Serial.begin(115200);
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
    input.trim(); // Trim the input first

    if (input.startsWith(CMD_ECHO)) {
        Serial.println(input.substring(CMD_ECHO.length()));
    } else if (input.startsWith(CMD_PARAM)) {
        String paramInput = input.substring(CMD_PARAM.length());
        paramInput.trim(); // Trim the parameter input
        handleParameterCommand(paramInput);
    } else if (input == CMD_SYS) {
        handleInfoCommand();
    } else if (input == CMD_SPEED) {
        handleSpeedCommand();
    } else if (input.startsWith(CMD_TRIP)) {
        String tripInput = input.substring(CMD_TRIP.length());
        tripInput.trim(); // Trim the trip input
        handleTripCommand(tripInput);
    } else if (input == CMD_RESET) {
        ESP.restart();
    } else if (input == "h" || input == CMD_HELP) {
        Serial.println(HELP_TEXT);
    } else {
        Serial.println("Unknown command. Type 'help' for a list of commands.");
    }
}


void handleSpeedCommand() {
    uint32_t currentSpeed = getSpeed();  // Assuming getSpeed() is accessible
    Serial.println("Current Speed: " + String(currentSpeed) + " Km/h");  // Adjust units if necessary
}

void handleInfoCommand() {
    Serial.println("System Information:");
    
    // Display number of tasks
    UBaseType_t uxTaskCount = uxTaskGetNumberOfTasks();
    Serial.print("Number of Tasks: ");
    Serial.println(uxTaskCount);

    // Display free heap size
    size_t xFreeHeapSize = xPortGetFreeHeapSize();
    Serial.print("Free Heap Size: ");
    Serial.println(xFreeHeapSize);

    // Display minimum free heap size
    size_t xMinimumEverFreeHeapSize = xPortGetMinimumEverFreeHeapSize();
    Serial.print("Minimum Ever Free Heap Size: ");
    Serial.println(xMinimumEverFreeHeapSize);

    // Display uptime
    uint32_t uptime = millis() / 1000;
    Serial.print("Uptime: ");
    Serial.print(uptime / 86400);
    Serial.print("d ");
    uptime %= 86400;  // Get remaining seconds after days
    if (uptime / 3600 < 10) Serial.print("0");
    Serial.print(uptime / 3600);
    Serial.print(":");
    if ((uptime % 3600) / 60 < 10) Serial.print("0");
    Serial.print((uptime % 3600) / 60);
    Serial.print(":");
    if (uptime % 60 < 10) Serial.print("0");
    Serial.println(uptime % 60);
}

void handleParameterCommand(String input) {
    if (input == "h" || input == CMD_HELP) {
        Serial.println(PARAM_HELP_TEXT);
    } else if (input.startsWith("clear")) {
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
    } else if (input.startsWith("update")) {
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
    } else if (input == "") {
        // List all parameters
        for(int i = 0; i < numParameters; i++) {
            Serial.println(String(parameters[i].index) + ": " + parameters[i].name + " = " + String(parameters[i].value));
        }
    } else {
        int valueIndex = input.indexOf(' ');
        if (valueIndex != -1) {
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

void handleTripCommand(String input) {
    if (input == "h" || input == CMD_HELP) {
        Serial.println(TRIP_HELP_TEXT);
    } else if (input.startsWith("r") || input.startsWith("reset")) {
        resetTripOdometer();
    } else if (input.startsWith("s") || input.startsWith("set")) {
        if (input.startsWith("set")) {
            input = input.substring(3);
        } else {
            input = input.substring(1);
        }
        String valueStr = input;
        valueStr.trim();
        if (valueStr.length() > 0) {
            if (!valueStr.toInt() && valueStr != "0") {
                Serial.println("Error: Invalid value");
                return;
            }
            setTripOdometer(valueStr.toInt() * 10);
        } else {
            Serial.println("Error: No value specified");
        }
    } else if (input == "") {
        char buffer[11];  // Buffer to hold formatted strings
        uint32_t tripOdometer = getTripOdometer();
        sprintf(buffer, "%03d.%d km", tripOdometer / 10, tripOdometer % 10);
        Serial.println(buffer);
    } else {
        Serial.println("Error: Invalid subcommand");
    }
}
