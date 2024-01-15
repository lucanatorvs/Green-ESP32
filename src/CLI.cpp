#include "CLI.h"
#include "Parameter.h"
#include "PulseCounterTask.h"
#include "GaugeControl.h"
#include "semaphores.h"
#include "DisplayTask.h"
#include "CANListenerTask.h"
#include "driveTelemetry.h"

// Predefine commands as constants for consistency and easy modification
const String CMD_HELP = "help";
const String CMD_ECHO = "echo ";
const String CMD_PARAM = "p";
const String CMD_SPEED = "s";
const String CMD_SYS = "sys";
const String CMD_TRIP = "trip";
const String CMD_RESET = "reset";
const String CMD_GAUGE = "g";
const String CMD_START_MONITOR = "canmonitor";
const String CMD_STOP_MONITOR = "stopmonitor";

// Command descriptions
const String HELP_TEXT = "Available commands:\n"
                         "  off                     - Turns off the cluster.\n"
                         "  on                      - Turns on the cluster.\n"
                         "  echo [text]             - Echoes the text back to the serial output.\n"
                         "  help                    - Displays this help message.\n"
                         "  p [subcommand]          - Parameter command. Type 'p help' for more information.\n"
                         "  reset                   - Resets the ESP32.\n"
                         "  ready                   - Displays the ready screen.\n"
                         "  s                       - Prints the current speed measurement.\n"
                         "  sys                     - Displays system information.\n"
                         "  trip [subcommand]       - Trip odometer command. Type 'trip help' for more information.\n"
                         "  canmonitor [id]         - Starts monitoring CAN messages. Type 'canmonitor help' for more info.\n"
                         "  stopmonitor             - Stops monitoring CAN messages.\n"
                         "  telemetry               - Displays the telemetry data.\n"
                         "  g [gauge_name] [pos]    - Gauge command. Type 'g help' for more information.";


const String PARAM_HELP_TEXT = "Usage: p [index] [value] | p update [index] | p clear [index]\n"
                               "  index: parameter index\n"
                               "  value: new value for parameter\n"
                               "  p update: update parameters from NVS\n"
                               "  p clear: clear NVS and reset parameters to default";

const String TRIP_HELP_TEXT = "Usage: trip [subcommand]\n"
                              "  subcommand: r, reset\n"
                              "  subcommand: s, set [value] km\n"
                              "  subcommand: help";

const String GAUGE_HELP_TEXT = "Usage: g [gauge_name] [position]\n"
                               "       g on  (turns all gauges on)\n"
                               "       g off (turns all gauges off)\n"
                               "       g autoupdate [on/off] (turns auto update on/off)\n"
                               "  gauge_name: Speedometer, Tachometer, Dynamometer, Chargeometer, Thermometer\n"
                               "  position: Position to set for the gauge\n"
                               "  Example: 'g Speedometer 50' sets the Speedometer to 50km/h\n";


void cliTask(void * parameter);
void handleInput(String input);
void handleParameterCommand(String input);
void handleSpeedCommand();
void handleInfoCommand();
void handleTripCommand(String input);
void handleGaugeCommand(String input);
void printTelemetryData();

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
                } else {
                    if (CANMonitoring() == 1) {
                        setCANMonitoring(false);
                        Serial.println("CAN monitoring stopped.");
                    }
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
    } else if (input == "ready") {
        currentDisplayMode = READY;
    } else if (input == "off") {
        currentDisplayMode = OFF;
        sendStandbyCommand(false);
    } else if (input == "on") {
        currentDisplayMode = EMPTY;
        sendStandbyCommand(true);
    } else if (input.startsWith(CMD_GAUGE)) {
        String gaugeInput = input.substring(CMD_GAUGE.length());
        gaugeInput.trim(); // Trim the gauge input
        handleGaugeCommand(gaugeInput);
    } else if (input == CMD_RESET) {
        ESP.restart();
    } else if (input == "telemetry") {
        printTelemetryData();
    } else if (input == "h" || input == CMD_HELP) {
        Serial.println(HELP_TEXT);
    } else if (input.startsWith(CMD_START_MONITOR)) {
        if (input == CMD_START_MONITOR + " h" || input == CMD_START_MONITOR + " help") {
            Serial.println("Usage: canmonitor [id]\n"
                           "  id: CAN ID to filter for (optional)");
            return;
        } else {
            String idStr = input.substring(CMD_START_MONITOR.length());
            idStr.trim();
            uint32_t filterID = (idStr.length() > 0) ? strtoul(idStr.c_str(), nullptr, 16) : 0;
            setCANMonitoring(true, filterID);
            Serial.println("CAN monitoring started.");
        }
    } else if (input == CMD_STOP_MONITOR) {
        setCANMonitoring(false);
        Serial.println("CAN monitoring stopped.");
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

    // the state of the semaphores
    UBaseType_t buttonSemaphoreCount = uxSemaphoreGetCount(buttonSemaphore);
    UBaseType_t buttonStateSemaphoreCount = uxSemaphoreGetCount(buttonStateSemaphore);
    Serial.print("Button Semaphore Count: ");
    Serial.println(buttonSemaphoreCount);
    Serial.print("Button State Semaphore Count: ");
    Serial.println(buttonStateSemaphoreCount);

    // give the current display mode
    Serial.print("Current Display Mode: ");
    switch (currentDisplayMode) {
        case EMPTY:
            Serial.println("EMPTY");
            break;
        case HELLO:
            Serial.println("HELLO");
            break;
        case NOTIFICATION:
            Serial.println("NOTIFICATION");
            break;
        case READY:
            Serial.println("READY");
            break;
        case OFF:
            Serial.println("OFF");
            break;
        default:
            Serial.println("UNKNOWN");
            break;
    }

    // give the auto update state of the gauges
    Serial.print("Auto Update: ");
    Serial.println(getAutoUpdate() ? "ON" : "OFF");
}

void printTelemetryData(){
    Serial.println("Telemetry Data:");

    Serial.print("Motor Temperature: ");
    Serial.println(telemetryData.motorTemp);

    Serial.print("Inverter Temperature: ");
    Serial.println(telemetryData.inverterTemp);

    Serial.print("Motor RPM: ");
    Serial.println(telemetryData.rpm);

    Serial.print("Motor DC Voltage: ");
    Serial.println(telemetryData.DCVoltage, 1); // 2 decimal places for float

    Serial.print("Motor DC Current: ");
    Serial.println(telemetryData.DCCurrent, 1); // 2 decimal places for float

    Serial.print("Power Unit Flags: ");
    String binaryString = String(telemetryData.powerUnitFlags, BIN);
    while (binaryString.length() < 16) {
        binaryString = "0" + binaryString;
    }
    Serial.println(binaryString);

    // Serial.print("Current: ");
    // Serial.println(telemetryData.Current / 10.0, 1); // Scaling factor for Current

    // Serial.print("Charge: ");
    // Serial.println(telemetryData.Charge / 10.0, 1); // Scaling factor for Charge

    Serial.print("State of Charge (SoC): ");
    Serial.println(telemetryData.SoC);

    // Serial.print("Voltage Limit: ");
    // Serial.println(telemetryData.VoltageLimit, 1);

    // Serial.print("Current Limit: ");
    // Serial.println(telemetryData.CurrentLimit, 1);

    // Serial.print("Charge Contactor State: ");
    // Serial.println(telemetryData.ChargeContactor ? "On" : "Off");

    // Serial.print("Battery Contactor State: ");
    // Serial.println(telemetryData.BatteryContactor ? "On" : "Off");

    // Serial.print("Voltage LSW: ");
    // Serial.println(telemetryData.VoltageLSW);

    // Serial.print("Voltage MSW: ");
    // Serial.println(telemetryData.VoltageMSW);

    // Serial.print("Estimated Energy: ");
    // Serial.println(telemetryData.EstimatedEnergy, 0);

    // Serial.print("High Temperature Warning: ");
    // Serial.println(telemetryData.HighTemperatureWarning ? "Yes" : "No");

    // Serial.print("Custom Current Limit: ");
    // Serial.println(telemetryData.CustomCurrentLimit);
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

void handleGaugeCommand(String input) {
    if (input == "h" || input == CMD_HELP) {
        Serial.println(GAUGE_HELP_TEXT);
    } else if (input == "on") {
        sendStandbyCommand(true);
    } else if (input == "off") {
        sendStandbyCommand(false);
    } else if (input.startsWith("autoupdate")) {
        String stateStr = input.substring(10);
        stateStr.trim();
        if (stateStr.equalsIgnoreCase("on")) {
            enableAutoUpdate(true);
        } else if (stateStr.equalsIgnoreCase("off")) {
            enableAutoUpdate(false);
        } else {
            Serial.println("Error: Invalid state. Please specify 'on' or 'off'.");
        }
    } else {
        int firstSpaceIndex = input.indexOf(' ');
        if (firstSpaceIndex == -1) {
            Serial.println("Error: Invalid gauge command. Please specify a gauge name and position.");
            return;
        }

        String gaugeName = input.substring(0, firstSpaceIndex);
        String positionStr = input.substring(firstSpaceIndex + 1);

        positionStr.trim();
        if (!positionStr.toInt() && positionStr != "0") {
            Serial.println("Error: Invalid position value. Please specify a position value, or use help for more information.");
            return;
        }

        int position = positionStr.toInt();

        if (gaugeName.equalsIgnoreCase("Speedometer")) {
            Speedometer.setPosition(position);
        } else if (gaugeName.equalsIgnoreCase("Tachometer")) {
            Tachometer.setPosition(position);
        } else if (gaugeName.equalsIgnoreCase("Dynamometer")) {
            Dynamometer.setPosition(position);
        } else if (gaugeName.equalsIgnoreCase("Chargeometer")) {
            Chargeometer.setPosition(position);
        } else if (gaugeName.equalsIgnoreCase("Thermometer")) {
            Thermometer.setPosition(position);
        } else {
            Serial.println("Error: Unknown gauge name. Available gauges: Speedometer, Tachometer, Dynamometer, Chargeometer, Thermometer.");
            return;
        }

        Serial.println("Gauge " + gaugeName + " set to position " + positionStr + ".");
    }
}
