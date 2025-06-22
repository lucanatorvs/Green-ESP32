#include "HelperTasks.h"
#include "DisplayTask.h"
#include "Bluetooth.h"
#include "PinAssignments.h"
#include "driveTelemetry.h"

// Function prototypes
void helperTask(void * parameter);
void manageBluetooth();
void manageLamps();

// External telemetry data reference
extern Telemetry telemetryData;

// Bluetooth management variables
DisplayMode previousDisplayMode = START;
bool btManualOverride = false;
unsigned long btLastActivityCheck = 0;
unsigned long btNoConnectionTimer = 0;
bool btTimerActive = false;

void initializeHelperTasks() {
    // Initialize lamp pins
    pinMode(BATTERY_Lamp_PIN, OUTPUT);
    pinMode(RUNNIG_Lamp_PIN, OUTPUT);
    pinMode(TEMPERATURE_Lamp_PIN, OUTPUT);
    pinMode(SOC_Lamp_PIN, OUTPUT);
    
    // Create the helper task
    xTaskCreate(
        helperTask,           // Task function
        "Helper Task",        // Name of task
        2048,                 // Stack size of task
        NULL,                 // Parameter of the task
        3,                    // Priority of the task
        NULL                  // Task handle to keep track of created task
    );
}

bool hasTimePassed(unsigned long &lastTime, unsigned long interval) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastTime >= interval) {
        lastTime = currentTime;
        return true;
    }
    
    return false;
}

void manageBluetooth() {
#ifdef ENABLE_BLUETOOTH
    // Check display mode changes
    if (currentDisplayMode == OFF && previousDisplayMode != OFF) {
        // Display just turned off, turn off Bluetooth
        Serial.println("Display turned OFF, turning off Bluetooth");
        turnBTOff();
        btTimerActive = false;  // Reset timer
    } 
    else if (previousDisplayMode == OFF && currentDisplayMode != OFF) {
        // Display just turned on from OFF, turn on Bluetooth
        Serial.println("Display turned ON, turning on Bluetooth");
        turnBTOn();
        // Start the no-connection timer
        btNoConnectionTimer = millis();
        btTimerActive = true;
    }
    
    // Check Bluetooth activity every minute when display is ON
    if (currentDisplayMode != OFF && hasTimePassed(btLastActivityCheck, 60000)) {  // 60 seconds
        if (isBTConnected()) {
            // There is a BT connection, keep Bluetooth on
            Serial.println("Bluetooth connection active, keeping Bluetooth on");
            // Reset the no-connection timer
            btNoConnectionTimer = millis();
            btTimerActive = true;
        } 
        else if (btTimerActive) {
            // No connection and timer is active
            unsigned long currentTime = millis();
            if (currentTime - btNoConnectionTimer >= 60000) {  // 60 seconds no connection
                Serial.println("No Bluetooth connection for 1 minute, turning off Bluetooth");
                turnBTOff();
                btTimerActive = false;
            }
        }
    }
    
    // Update previous display mode
    previousDisplayMode = currentDisplayMode;
#endif // ENABLE_BLUETOOTH
}

// This is where you implement your custom periodic logic
void periodicFunction() {
#ifdef ENABLE_BLUETOOTH
    // Manage Bluetooth based on display state and activity
    manageBluetooth();
#endif
    
    // Manage all indicator lamps
    manageLamps();
    
    // Add other periodic functions here
}

// Function to manage all indicator lamps
void manageLamps() {
    // Battery lamp control
    if (telemetryData.BMSChargingState != 0) {
        digitalWrite(BATTERY_Lamp_PIN, HIGH); // Turn on the battery lamp if charging
    } else {
        digitalWrite(BATTERY_Lamp_PIN, LOW); // Turn off the battery lamp if not charging
    }
    
    // Temperature lamp control
    int8_t maxTempMotor = max(telemetryData.motorTemp, telemetryData.inverterTemp);
    int8_t maxTemp = max(maxTempMotor, telemetryData.BMSMaxModTemp);
    int gaugeTemp = max(maxTemp, telemetryData.BMSMaxCellTemp);
    if (telemetryData.BMSMinCellTemp <= 2) {
        gaugeTemp = telemetryData.BMSMinCellTemp;
    }
    
    if (gaugeTemp > 70) {
        digitalWrite(TEMPERATURE_Lamp_PIN, HIGH); // Turn on temperature lamp if gauge temperature is above 70C
    } else {
        digitalWrite(TEMPERATURE_Lamp_PIN, LOW); // Turn off temperature lamp
    }
    
    // SoC lamp control
    if (telemetryData.SoC < 20) {
        digitalWrite(SOC_Lamp_PIN, HIGH); // Turn on SOC lamp if SoC is below 20%
    } else {
        digitalWrite(SOC_Lamp_PIN, LOW); // Turn off SOC lamp
    }
    
    // Running lamp is controlled by motor status via setRunningLamp()
}

// Function to control the running lamp state
void setRunningLamp(bool state) {
    digitalWrite(RUNNIG_Lamp_PIN, state ? HIGH : LOW);
}

void helperTask(void * parameter) {
    for (;;) {
        // Call the single periodic function
        periodicFunction();
        
        // Delay for 10ms as requested
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
