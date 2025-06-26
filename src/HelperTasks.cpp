#include "HelperTasks.h"
#include "DisplayTask.h"
#include "Bluetooth.h"
#include "PinAssignments.h"
#include "driveTelemetry.h"

// Function prototypes
void helperTask(void * parameter);
void manageBluetooth();
void manageLamps();
void updateSmoothedValues(float instantPower, float instantSpeed, float voltage);
void manageBrakeLight();

// External telemetry data reference
extern Telemetry telemetryData;

// Bluetooth management variables
DisplayMode previousDisplayMode = START;
bool btManualOverride = false;
unsigned long btLastActivityCheck = 0;
unsigned long btNoConnectionTimer = 0;
bool btTimerActive = false;

// Smoothed telemetry values using exponential moving average
#define EMA_ALPHA 0.002    // Lower alpha for much slower changes (approximately ~500 samples influence)
float smoothedConsumption = 0;    // Smoothed Wh/km value
int smoothedRange = 0;            // Smoothed range in km
bool isRegenerating = false;      // Flag to indicate regeneration state
float lastValidConsumption = 0;   // Last valid consumption when speed > 2 km/h
int lastValidRange = 0;           // Last valid range when speed > 2 km/h
unsigned long lastValueUpdate = 0; // Last time the values were updated
bool valuesInitialized = false;   // Flag to track if values are initialized

// Time between smoothed value updates (ms)
#define VALUE_UPDATE_INTERVAL 100  // 10Hz update rate

void initializeHelperTasks() {
    // Initialize lamp pins
    pinMode(BATTERY_Lamp_PIN, OUTPUT);
    pinMode(RUNNIG_Lamp_PIN, OUTPUT);
    pinMode(TEMPERATURE_Lamp_PIN, OUTPUT);
    pinMode(SOC_Lamp_PIN, OUTPUT);
    
    // // Initialize brake light pin
    // pinMode(BRAKE_LIGHT_PIN, OUTPUT);
    // digitalWrite(BRAKE_LIGHT_PIN, LOW); // Make sure it's off initially
    
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
    
    // Manage the brake light
    // manageBrakeLight();
    
    // Update smoothed telemetry values
    if (currentDisplayMode != OFF && hasTimePassed(lastValueUpdate, VALUE_UPDATE_INTERVAL)) {
        float voltage = telemetryData.DCVoltage;
        float current = telemetryData.DCCurrent;
        float speed = telemetryData.speed;
        float power = voltage * current; // Watts
        
        updateSmoothedValues(power, speed, voltage);
    }
    
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

// Function to manage the brake light based on regenerative power
void manageBrakeLight() {
    // // Calculate current power (in Watts)
    // float voltage = telemetryData.DCVoltage;
    // float current = telemetryData.DCCurrent;
    // float power = voltage * current;
    
    // // Turn on brake light when regenerative power is >= 10kW (negative power)
    // // Note: regenerative braking produces negative power (negative current)
    // if (power <= -100) { // Power <= -10kW (100W)
    //     digitalWrite(BRAKE_LIGHT_PIN, HIGH); // Turn on brake light
    // } else {
    //     digitalWrite(BRAKE_LIGHT_PIN, LOW); // Turn off brake light
    // }
}

void helperTask(void * parameter) {
    for (;;) {
        // Call the single periodic function
        periodicFunction();
        
        // Delay for 10ms as requested
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// Function to update smoothed values for consumption and range
void updateSmoothedValues(float instantPower, float instantSpeed, float voltage) {
    // Check if we're regenerating
    isRegenerating = (instantPower < 0);
    
    // When speed is below 2 km/h, maintain the last valid values
    if (instantSpeed < 2.0) {
        // Use last valid values if already initialized
        if (valuesInitialized) {
            smoothedConsumption = lastValidConsumption;
            
            // Set range to 999 when regenerating, otherwise use last valid range
            if (isRegenerating) {
                smoothedRange = 999;
            } else {
                smoothedRange = lastValidRange;
            }
        }
        return;
    }

    // Calculate current consumption value
    float instantConsumption = 0;
    int instantRange = 0;
    
    if (instantSpeed >= 2.0) {
        // Always use absolute power for consumption calculation
        instantConsumption = fabs(instantPower / instantSpeed); // Wh/km
        
        // If regenerating, set range to 999 (unlimited)
        if (isRegenerating) {
            instantRange = 999;
        } else {
            // Calculate instant range when discharging
            float chargeAh = telemetryData.Charge / 10.0f;
            instantRange = (int)((chargeAh * voltage) / instantConsumption);
            
            // Cap range values
            if (instantRange > 999) instantRange = 999;
            if (instantRange < 0) instantRange = 0;
        }
        
        // Initialize values if needed
        if (!valuesInitialized) {
            smoothedConsumption = instantConsumption;
            smoothedRange = instantRange;
            lastValidConsumption = instantConsumption;
            lastValidRange = instantRange;
            valuesInitialized = true;
        } else {
            // Apply exponential moving average (EMA) formula: 
            // newValue = alpha * currentValue + (1 - alpha) * previousValue
            smoothedConsumption = EMA_ALPHA * instantConsumption + (1 - EMA_ALPHA) * smoothedConsumption;
            
            // For range, we handle differently for regeneration vs discharge
            if (isRegenerating) {
                smoothedRange = 999; // Always max range when regenerating
            } else {
                smoothedRange = EMA_ALPHA * instantRange + (1 - EMA_ALPHA) * smoothedRange;
            }
            
            // Store current values as last valid values, but only for discharge
            if (!isRegenerating) {
                lastValidConsumption = smoothedConsumption;
                lastValidRange = smoothedRange;
            }
        }
    }
}

// Function to get smoothed consumption value
float getSmoothedConsumption() {
    return smoothedConsumption;
}

// Function to get smoothed range value
int getSmoothedRange() {
    return smoothedRange;
}

// Function to check if currently regenerating
bool isRegeneratingPower() {
    return isRegenerating;
}
