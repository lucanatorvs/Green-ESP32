#include "CLI.h"
#include "Parameter.h"
#include "PulseCounterTask.h"
#include "GaugeControl.h"
#include "Semaphores.h"
#include "DisplayTask.h"
#include "CANListenerTask.h"
#include "driveTelemetry.h"
#include "OTA.h"
#include <Preferences.h>

// WiFi namespace constants from OTA.cpp
#define WIFI_NAMESPACE "wifi"
#define WIFI_SSID_KEY "ssid"
#define WIFI_PASS_KEY "password"
#define WIFI_ENABLED_KEY "enabled"

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
const String CMD_WIFI = "wifi";

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
                         "  g [gauge_name] [pos]    - Gauge command. Type 'g help' for more information.\n"
                         "  wifi [subcommand]       - WiFi command. Type 'wifi help' for more information.";


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

const String WIFI_HELP_TEXT = "Usage: wifi [subcommand]\n"
                              "  subcommand: status   - Shows WiFi connection status\n"
                              "  subcommand: restart  - Restarts WiFi connection\n"
                              "  subcommand: on       - Enables WiFi\n"
                              "  subcommand: off      - Disables WiFi\n"
                              "  subcommand: ssid <name>     - Sets the WiFi SSID\n"
                              "  subcommand: password <pass> - Sets the WiFi password\n"
                              "  subcommand: help     - Shows this help message\n";


void cliTask(void * parameter);
void processCharacter(char ch, String &input, Stream &stream);
void handleInput(String input, Stream &stream);
void handleParameterCommand(String input, Stream &stream);
void handleSpeedCommand(Stream &stream);
void handleInfoCommand(Stream &stream);
void handleTripCommand(String input, Stream &stream);
void handleGaugeCommand(String input, Stream &stream);
void handleWiFiCommand(String input, Stream &stream);
void printTelemetryData(Stream &stream);

void initializeCLI() {
    Serial.begin(115200);
    xTaskCreate(cliTask, "CLI Task", 4096, NULL, 2, NULL);
}

void cliTask(void * parameter) {
    String input;
    for (;;) {
        // Check for input from hardware Serial
        if (Serial.available() > 0) {
            char ch = Serial.read();
            processCharacter(ch, input, Serial);
        }
        
        // Check for input from Bluetooth Serial
        if (SerialBT.available() > 0) {
            char ch = SerialBT.read();
            processCharacter(ch, input, SerialBT);
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS); // Wait 10ms
    }
}

void processCharacter(char ch, String &input, Stream &stream) {
    if (ch == '\b' || ch == 127) {  // ASCII for backspace or delete
        if (input.length() > 0) {
            input.remove(input.length() - 1);  // Remove last character from input
            stream.write(127);  // Send DEL character
        }
    } else if (ch == '\n' || ch == '\r') {  // New line or carriage return
        stream.println();  // Echo new line back to user immediately
        if (input.length() > 0) {  // Only process non-empty commands
            input.trim();
            handleInput(input, stream);
            input = "";  // Reset input string for next command
        } else {
            if (CANMonitoring() == 1) {
                setCANMonitoring(false);
                stream.println("CAN monitoring stopped.");
            }
        }
    } else {
        input += ch;  // Accumulate characters into input string
        stream.print(ch);  // Echo character back to user
    }
}

void handleInput(String input, Stream &stream) {
    input.trim(); // Trim the input first

    if (input.startsWith(CMD_ECHO)) {
        stream.println(input.substring(CMD_ECHO.length()));
    } else if (input.startsWith(CMD_PARAM)) {
        String paramInput = input.substring(CMD_PARAM.length());
        paramInput.trim(); // Trim the parameter input
        handleParameterCommand(paramInput, stream);
    } else if (input == CMD_SYS) {
        handleInfoCommand(stream);
    } else if (input == CMD_SPEED) {
        handleSpeedCommand(stream);
    } else if (input.startsWith(CMD_TRIP)) {
        String tripInput = input.substring(CMD_TRIP.length());
        tripInput.trim(); // Trim the trip input
        handleTripCommand(tripInput, stream);
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
        handleGaugeCommand(gaugeInput, stream);
    } else if (input == CMD_RESET) {
        stream.println("Restarting ESP32...");
        ESP.restart();
    } else if (input == "telemetry") {
        printTelemetryData(stream);
    } else if (input == "h" || input == CMD_HELP) {
        stream.println(HELP_TEXT);
    } else if (input.startsWith(CMD_START_MONITOR)) {
        if (input == CMD_START_MONITOR + " h" || input == CMD_START_MONITOR + " help") {
            stream.println("Usage: canmonitor [id]\n"
                           "  id: CAN ID to filter for (optional)");
            return;
        } else {
            String idStr = input.substring(CMD_START_MONITOR.length());
            idStr.trim();
            uint32_t filterID = (idStr.length() > 0) ? strtoul(idStr.c_str(), nullptr, 16) : 0;
            setCANMonitoring(true, filterID);
            stream.print("CAN monitoring started.");
            if (filterID != 0) {
                stream.print(" Filtering for hex ID: ");
                stream.print(filterID, HEX);
            }
            stream.println();
        }
    } else if (input == CMD_STOP_MONITOR) {
        setCANMonitoring(false);
        stream.println("CAN monitoring stopped.");
    } else if (input.startsWith(CMD_WIFI)) {
        String wifiInput = input.substring(CMD_WIFI.length());
        wifiInput.trim(); // Trim the wifi input
        handleWiFiCommand(wifiInput, stream);
    } else {
        stream.println("Unknown command. Type 'help' for a list of commands.");
    }
}

void handleSpeedCommand(Stream &stream) {
    uint32_t currentSpeed = getSpeed();  // Assuming getSpeed() is accessible
    stream.println("Current Speed: " + String(currentSpeed) + " Km/h");  // Adjust units if necessary
}

void handleInfoCommand(Stream &stream) {
    stream.println("System Information:");
    
    // Display number of tasks
    UBaseType_t uxTaskCount = uxTaskGetNumberOfTasks();
    stream.print("Number of Tasks: ");
    stream.println(uxTaskCount);

    // Display free heap size
    size_t xFreeHeapSize = xPortGetFreeHeapSize();
    stream.print("Free Heap Size: ");
    stream.println(xFreeHeapSize);

    // Display minimum free heap size
    size_t xMinimumEverFreeHeapSize = xPortGetMinimumEverFreeHeapSize();
    stream.print("Minimum Ever Free Heap Size: ");
    stream.println(xMinimumEverFreeHeapSize);

    // Display uptime
    uint32_t uptime = millis() / 1000;
    stream.print("Uptime: ");
    stream.print(uptime / 86400);
    stream.print("d ");
    uptime %= 86400;  // Get remaining seconds after days
    if (uptime / 3600 < 10) stream.print("0");
    stream.print(uptime / 3600);
    stream.print(":");
    if ((uptime % 3600) / 60 < 10) stream.print("0");
    stream.print((uptime % 3600) / 60);
    stream.print(":");
    if (uptime % 60 < 10) stream.print("0");
    stream.println(uptime % 60);

    // the state of the semaphores
    UBaseType_t buttonSemaphoreCount = uxSemaphoreGetCount(buttonSemaphore);
    UBaseType_t buttonStateSemaphoreCount = uxSemaphoreGetCount(buttonStateSemaphore);
    stream.print("Button Semaphore Count: ");
    stream.println(buttonSemaphoreCount);
    stream.print("Button State Semaphore Count: ");
    stream.println(buttonStateSemaphoreCount);

    // give the current display mode
    stream.print("Current Display Mode: ");
    switch (currentDisplayMode) {
        case EMPTY:
            stream.println("EMPTY");
            break;
        case HELLO:
            stream.println("HELLO");
            break;
        case SOC:
            stream.println("SOC");
            break;
        case SPEED:
            stream.println("SPEED");
            break;
        case NOTIFICATION:
            stream.println("NOTIFICATION");
            break;
        case READY:
            stream.println("READY");
            break;
        case OFF:
            stream.println("OFF");
            break;
        default:
            stream.println("UNKNOWN");
            break;
    }

    // give the auto update state of the gauges
    stream.print("Auto Update: ");
    stream.println(getAutoUpdate() ? "ON" : "OFF");
    
    // WiFi status
    stream.print("WiFi Status: ");
    stream.println(getWiFiStatusString());
    
    // Bluetooth status
    stream.print("Bluetooth Connected: ");
    stream.println(isBTConnected() ? "Yes" : "No");
}

void printTelemetryData(Stream &stream) {
    String binaryString;
    stream.println("Telemetry Data:");

    stream.print("Speed: ........................ ");
    stream.println(telemetryData.speed / 10.0, 1);

    stream.print("Motor Temperature: ............ ");
    stream.println(telemetryData.motorTemp);

    stream.print("Inverter Temperature: ......... ");
    stream.println(telemetryData.inverterTemp);

    stream.print("Motor RPM: .................... ");
    stream.println(telemetryData.rpm);

    stream.print("Motor DC Voltage: ............. ");
    stream.println(telemetryData.DCVoltage, 1); // 2 decimal places for float

    stream.print("Motor DC Current: ............. ");
    stream.println(telemetryData.DCCurrent, 1); // 2 decimal places for float

    stream.print("Power Unit Flags: ............. ");
    binaryString = String(telemetryData.powerUnitFlags, BIN);
    while (binaryString.length() < 16) {
        binaryString = "0" + binaryString;
    }
    stream.println(binaryString);

    stream.print("BMS Input Signal Flags: ....... ");
    binaryString = String(telemetryData.BMSInputSignalFlags, BIN);
    while (binaryString.length() < 8) {
        binaryString = "0" + binaryString;
    }
    stream.println(binaryString);

    stream.print("BMS Output Signal Flags: ...... ");
    binaryString = String(telemetryData.BMSOutputSignalFlags, BIN);
    while (binaryString.length() < 8) {
        binaryString = "0" + binaryString;
    }
    stream.println(binaryString);

    stream.print("BMS Number of Cells: .......... ");
    stream.println(telemetryData.BMSNumberOfCells);


    stream.print("BMS Charging State: ........... ");
    switch (telemetryData.BMSChargingState) {
        case 0:
            stream.println("Disconnected");
            break;
        case 1:
            stream.println("Pre-heat");
            break;
        case 2:
            stream.println("Pre-charge");
            break;
        case 3:
            stream.println("Charging");
            break;
        case 4:
            stream.println("Balancing");
            break;
        case 5:
            stream.println("Finished");
            break;
        case 6:
            stream.println("Error");
            break;
        default:
            stream.println("Unknown");
            break;
    }

    stream.print("BMS Charging State Duration: .. ");
    stream.print(telemetryData.BMSCsDuration);
    stream.println(" minutes");

    stream.print("BMS Last Charging Error: ...... ");
    switch (telemetryData.BMSLastChargingError) {
        case 0:
            stream.println("No error");
            break;
        case 1:
            stream.println("No cell comm. at start/precharge (CAN charger)");
            break;
        case 2:
            stream.println("No cell comm. (Non-CAN charger)");
            break;
        case 3:
            stream.println("Max charging stage duration expired");
            break;
        case 4:
            stream.println("Cell comm. lost during charging/balancing (CAN charger)");
            break;
        case 5:
            stream.println("Cannot set balancing threshold");
            break;
        case 6:
            stream.println("Cell/module temp too high");
            break;
        case 7:
            stream.println("Cell comm. lost during pre-heating (CAN charger)");
            break;
        case 8:
            stream.println("Cell count mismatch");
            break;
        case 9:
            stream.println("Cell over-voltage");
            break;
        case 10:
            stream.println("Cell protection event (see diagnostic codes)");
            break;
        default:
            stream.println("Unknown");
            break;
    }

    stream.print("BMS Protection Flags: ......... ");
    binaryString = String(telemetryData.BMSProtectionFlags, BIN);
    while (binaryString.length() < 32) {
        binaryString = "0" + binaryString;
    }
    stream.println(binaryString);

    stream.print("BMS Reduction Flags: .......... ");
    binaryString = String(telemetryData.BMSReductionFlags, BIN);
    while (binaryString.length() < 8) {
        binaryString = "0" + binaryString;
    }
    stream.println(binaryString);

    stream.print("BMS Battery Status Flags: ..... ");
    binaryString = String(telemetryData.BMSBatteryStatusFlags, BIN);
    while (binaryString.length() < 8) {
        binaryString = "0" + binaryString;
    }
    stream.println(binaryString);

    stream.print("BMS Minimum Module Temperature: ");
    stream.println(telemetryData.BMSMinModTemp);
    stream.print("BMS Maximum Module Temperature: ");
    stream.println(telemetryData.BMSMaxModTemp);
    stream.print("BMS Average Module Temperature: ");
    stream.println(telemetryData.BMSAverageModTemp);
    stream.print("BMS Minimum Cell Temperature: . ");
    stream.println(telemetryData.BMSMinCellTemp);
    stream.print("BMS Maximum Cell Temperature: . ");
    stream.println(telemetryData.BMSMaxCellTemp);
    stream.print("BMS Average Cell Temperature: . ");
    stream.println(telemetryData.BMSAverageCellTemp);

    stream.print("BMS Current: .................. ");
    stream.println(telemetryData.Current / 10.0, 1);

    stream.print("BMS Charge: ................... ");
    stream.println(telemetryData.Charge / 10.0, 1);

    stream.print("BMS State of Charge (SoC): .... ");
    stream.println(telemetryData.SoC / 100.0, 2);
}

void handleParameterCommand(String input, Stream &stream) {
    if (input == "h" || input == CMD_HELP) {
        stream.println(PARAM_HELP_TEXT);
    } else if (input.startsWith("clear")) {
        String indexStr = input.substring(6);
        indexStr.trim();
        if (indexStr.length() > 0) {
            if (!indexStr.toInt() && indexStr != "0") {
                stream.println("Error: Invalid index");
                return;
            }
            clearNVS(indexStr.toInt(), &stream);
        } else {
            clearNVS(-1, &stream);
        }
    } else if (input.startsWith("update")) {
        String indexStr = input.substring(7);
        indexStr.trim();
        if (indexStr.length() > 0) {
            if (!indexStr.toInt() && indexStr != "0") {
                stream.println("Error: Invalid index");
                return;
            }
            updateParametersFromNVS(indexStr.toInt(), &stream);
        } else {
            updateParametersFromNVS(-1, &stream);
        }
    } else if (input == "") {
        // List all parameters
        for(int i = 0; i < numParameters; i++) {
            stream.println(String(parameters[i].index) + ": " + parameters[i].name + " = " + String(parameters[i].value));
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
                stream.println("Error: Invalid index");
                return;
            }
            if (!valueStr.toInt() && valueStr != "0") {
                stream.println("Error: Invalid value");
                return;
            }
            int index = indexStr.toInt();
            int value = valueStr.toInt();
            setParameter(index, value, &stream);
        } else {
            // Check for invalid index
            if (!input.toInt() && input != "0") {
                stream.println("Error: Invalid index");
                return;
            }
            int index = input.toInt();
            getParameter(index, &stream);
        }
    }
}

void handleTripCommand(String input, Stream &stream) {
    if (input == "h" || input == CMD_HELP) {
        stream.println(TRIP_HELP_TEXT);
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
                stream.println("Error: Invalid value");
                return;
            }
            setTripOdometer(valueStr.toInt() * 10);
        } else {
            stream.println("Error: No value specified");
        }
    } else if (input == "") {
        char buffer[11];  // Buffer to hold formatted strings
        uint32_t tripOdometer = getTripOdometer();
        sprintf(buffer, "%03d.%d km", tripOdometer / 10, tripOdometer % 10);
        stream.println(buffer);
    } else {
        stream.println("Error: Invalid subcommand");
    }
}

void handleGaugeCommand(String input, Stream &stream) {
    if (input == "h" || input == CMD_HELP) {
        stream.println(GAUGE_HELP_TEXT);
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
            stream.println("Error: Invalid state. Please specify 'on' or 'off'.");
        }
    } else {
        int firstSpaceIndex = input.indexOf(' ');
        if (firstSpaceIndex == -1) {
            stream.println("Error: Invalid gauge command. Please specify a gauge name and position.");
            return;
        }

        String gaugeName = input.substring(0, firstSpaceIndex);
        String positionStr = input.substring(firstSpaceIndex + 1);

        positionStr.trim();
        if (!positionStr.toInt() && positionStr != "0") {
            stream.println("Error: Invalid position value. Please specify a position value, or use help for more information.");
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
            stream.println("Error: Unknown gauge name. Available gauges: Speedometer, Tachometer, Dynamometer, Chargeometer, Thermometer.");
            return;
        }

        stream.println("Gauge " + gaugeName + " set to position " + positionStr + ".");
    }
}

void handleWiFiCommand(String input, Stream &stream) {
    if (input == "h" || input == "help") {
        stream.println(WIFI_HELP_TEXT);
    } else if (input == "status") {
        stream.println("WiFi Status: " + getWiFiStatusString());
        stream.println("WiFi Enabled: " + String(isWiFiEnabled() ? "Yes" : "No"));
        
        // Show SSID if available
        String ssid = getWiFiSSID();
        if (ssid.length() > 0) {
            stream.println("SSID: " + ssid);
        } else {
            stream.println("SSID: Not set");
        }
    } else if (input == "restart") {
        stream.println("Restarting WiFi connection...");
        bool result = startWiFi();
        if (result) {
            stream.println("WiFi restarted successfully.");
        } else {
            stream.println("WiFi restart failed. Check your WiFi parameters.");
        }
    } else if (input == "on") {
        stream.println("Enabling WiFi...");
        bool result = setWiFiEnabled(true);
        if (result) {
            stream.println("WiFi enabled successfully.");
        } else {
            stream.println("Failed to enable WiFi. Check your WiFi parameters.");
        }
    } else if (input == "off") {
        stream.println("Disabling WiFi...");
        bool result = setWiFiEnabled(false);
        if (result) {
            stream.println("WiFi disabled successfully.");
        } else {
            stream.println("Failed to disable WiFi.");
        }
    } else if (input.startsWith("ssid ")) {
        String ssid = input.substring(5); // Extract the SSID after "ssid "
        ssid.trim();
        
        if (ssid.length() == 0) {
            stream.println("Error: SSID cannot be empty");
            return;
        }
        
        // Store current password
        String currentPassword = "";
        Preferences preferences;
        preferences.begin(WIFI_NAMESPACE, true); // Read-only mode
        currentPassword = preferences.getString(WIFI_PASS_KEY, "");
        preferences.end();
        
        // Set the new SSID with existing password
        bool result = setWiFiCredentials(ssid, currentPassword);
        if (result) {
            stream.println("WiFi SSID set to: " + ssid);
        } else {
            stream.println("Failed to set WiFi SSID");
        }
    } else if (input.startsWith("password ")) {
        String password = input.substring(9); // Extract the password after "password "
        password.trim();
        
        // Get current SSID
        String currentSSID = getWiFiSSID();
        if (currentSSID.length() == 0) {
            stream.println("Error: SSID not set. Please set SSID first using 'wifi ssid <name>'");
            return;
        }
        
        // Set the SSID with new password
        bool result = setWiFiCredentials(currentSSID, password);
        if (result) {
            stream.println("WiFi password set successfully");
        } else {
            stream.println("Failed to set WiFi password");
        }
    } else {
        stream.println("Error: Invalid WiFi command. Type 'wifi help' for more information.");
    }
}
