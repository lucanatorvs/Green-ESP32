#include "DisplayTask.h"
#include "Parameter.h"
#include "PinAssignments.h"
#include "PulseCounterTask.h"
#include "Semaphores.h"
#include "driveTelemetry.h"
#include "GaugeControl.h"
#include "Bluetooth.h"
#include "HelperTasks.h"

#define X1 4    // x coordinate of the top left corner of the odometer
#define Y1 12   // y coordinate of the top left corner of the odometer
#define X2 123  // x coordinate of the bottom right corner of the odometer
#define Y2 50   // y coordinate of the bottom right corner of the odometer

U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI display(U8G2_R0, DISPLAY_CHIP_SELECT_PIN, DISPLAY_DATA_COMMAND_PIN, DISPLAY_RESET_PIN);

// Ignition control variables
bool ignitionOverrideEnabled = false;
bool manualIgnitionState = false;

void displayTask(void * parameter);
void displayModeSwichTask(void * parameter);
void turnOnTask(void * parameter);
void drawOdometer();

// Helper function to calculate range and consumption
void calculateConsumptionAndRange(int &rangeInt, int &usageInt) {
    // If BMSConsumptionEstimate is valid, use it
    if (telemetryData.BMSConsumptionEstimate != 0xFFFF && telemetryData.BMSConsumptionEstimate != 0) {
        usageInt = telemetryData.BMSConsumptionEstimate;
        if (usageInt < 0) usageInt = 0;
        rangeInt = (telemetryData.Charge * 10) / usageInt;
    } else {
        // Use smoothed values for more stable display
        usageInt = (int)getSmoothedConsumption();
        rangeInt = getSmoothedRange();
        
        // Fallback if smoothed values are not available yet
        if (usageInt == 0 && rangeInt == 0) {
            float voltage = telemetryData.DCVoltage;
            float current = telemetryData.DCCurrent;
            float speed = telemetryData.speed;
            
            if (speed > 1.0) {
                float power = voltage * current; // Watts
                float consumption = fabs(power / speed); // Wh/km, always positive
                usageInt = (int)consumption;
                if (usageInt < 1) usageInt = 1; // Avoid div by zero
                
                // Set range based on regeneration status
                if (current <= 0.0) { // Regenerating
                    rangeInt = 999;
                } else { // Discharging
                    float chargeAh = telemetryData.Charge / 10.0f;
                    float range = (chargeAh * voltage) / consumption;
                    rangeInt = (int)range;
                }
            } else if (current <= 0.0) {
                // Regenerating or not consuming, cap range
                rangeInt = 999;
                usageInt = 0;
            }
        }
    }
    
    // Cap range values
    if (rangeInt > 999) rangeInt = 999;
    if (rangeInt < 0) rangeInt = 0;
}

// Ignition override control functions
void setIgnitionOverride(bool enabled) {
    ignitionOverrideEnabled = enabled;
    Serial.println(enabled ? "Ignition override ENABLED" : "Ignition override DISABLED");
}

bool getIgnitionOverride() {
    return ignitionOverrideEnabled;
}

void setIgnitionState(bool on) {
    manualIgnitionState = on;
    // The actual state change is handled in the turnOnTask
}

bool getIgnitionState() {
    return manualIgnitionState;
}

DisplayMode currentDisplayMode = EMPTY; // Global variable to keep track of the current display mode

void initializeDisplayTask() {
    if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
        display.begin();
        vTaskDelay(pdMS_TO_TICKS(50));
        display.enableUTF8Print();

        xSemaphoreGive(spiBusMutex);
    }

    xTaskCreate(displayTask, "Display Task", 4096, NULL, 1, NULL);
    xTaskCreate(displayModeSwichTask, "Display Mode Switch Task", 2048, NULL, 2, NULL);
    xTaskCreate(turnOnTask, "Turn On Task", 2048, NULL,3, NULL);
}

void turnOnTask(void * parameter) {
    // Set the pinmode for the IGNITION_SWITCH_PIN to input (not pullup)
    pinMode(IGNITION_SWITCH_PIN, INPUT);
    vTaskDelay(pdMS_TO_TICKS(200));
    
    // Define analog threshold value - adjust as needed based on testing
    const int ANALOG_THRESHOLD = 500; // Using 500 as a starting threshold (0-4095 range)
    
    for (;;) {
        if (ignitionOverrideEnabled) {
            // Use manual ignition state instead of reading the pin
            if (manualIgnitionState && currentDisplayMode == OFF) {
                currentDisplayMode = EMPTY;
                sendStandbyCommand(true);
                Serial.println("Manual ignition ON");
            } else if (!manualIgnitionState && currentDisplayMode != OFF) {
                currentDisplayMode = OFF;
                sendStandbyCommand(false);
                Serial.println("Manual ignition OFF");
            }
        } else {
            // Normal operation - read the analog value of the pin
            int analogValue = analogRead(IGNITION_SWITCH_PIN);
            
            // Use the analog value with a threshold to determine state
            if (currentDisplayMode == OFF && analogValue > ANALOG_THRESHOLD) {
                currentDisplayMode = EMPTY;
                sendStandbyCommand(true);
            } else if (analogValue <= ANALOG_THRESHOLD && currentDisplayMode != OFF) {
                currentDisplayMode = OFF;
                sendStandbyCommand(false);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void displayModeSwichTask(void * parameter) {
    for (;;) {
        if (xSemaphoreTake(buttonStateSemaphore, portMAX_DELAY) == pdTRUE) {
            if (currentDisplayMode != OFF) {
                if (currentDisplayMode == NOTIFICATION || currentDisplayMode == READY) {
                    currentDisplayMode = EMPTY; // Dismiss the notification
                } else {
                    if (currentDisplayMode == EMPTY) {
                        currentDisplayMode = START;
                    } else  if (currentDisplayMode == START) {
                        currentDisplayMode = SOC;
                    } else if (currentDisplayMode == SOC) {
                        currentDisplayMode = SPEED;
                    } else {
                        currentDisplayMode = EMPTY;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // Polling delay to avoid busy-waiting
    }
}

void displayTask(void * parameter) {
    for (;;) {
        if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
            if (currentDisplayMode != OFF) {
                // small delay to allow display to power up
                vTaskDelay(pdMS_TO_TICKS(50));

                display.enableUTF8Print();
                display.clearBuffer();
                drawOdometer();

                // Draw content based on the current display mode
                switch (currentDisplayMode) {
                    case EMPTY:
                        // Nothing to draw in this mode
                        break;
                    case START:
                    {
                        display.setFont(u8g2_font_6x12_tf);
                        int rangeInt = 0, usageInt = 0;
                        calculateConsumptionAndRange(rangeInt, usageInt);
                        display.drawStr(X1 + 3, Y1 + 20, "Range: ");
                        display.drawStr(X1 + 3 + display.getStrWidth("Range: "), Y1 + 20, String(rangeInt).c_str());
                        display.drawStr(X1 + 3 + display.getStrWidth("Range: ") + display.getStrWidth(String(rangeInt).c_str()) + 4, Y1 + 20, "km");
                        
                        // Change label from "Usage" to "Regen" when regenerating
                        bool isRegen = isRegeneratingPower();
                        const char* label = isRegen ? "Regen: " : "Usage: ";
                        display.drawStr(X1 + 3, Y1 + 32, label);
                        int usageX = X1 + 3 + display.getStrWidth(label);
                        
                        // Just display the value without a sign
                        display.drawStr(usageX, Y1 + 32, String(usageInt).c_str());
                        display.drawStr(usageX + display.getStrWidth(String(usageInt).c_str()) + 4, Y1 + 32, "Wh/km");
                        break;
                    }
                    case SOC:
                        display.setFont(u8g2_font_6x12_tf);
                        // draw SoC: telemetryData.SoC%
                        display.drawStr(X1 + 3, Y1 + 20, "SoC: ");
                        display.drawStr(X1 + 3 + display.getStrWidth("SoC: "), Y1 + 20, String(telemetryData.SoC).c_str());
                        display.drawStr(X1 + 3 + display.getStrWidth("SoC: ") + display.getStrWidth(String(telemetryData.SoC).c_str()) + 1, Y1 + 20, "%");
                        // next line is battery voltage
                        display.drawStr(X1 + 3, Y1 + 32, "Vbat: ");
                        display.drawStr(X1 + 3 + display.getStrWidth("Vbat: "), Y1 + 32, String(telemetryData.DCVoltage).c_str());
                        display.drawStr(X1 + 3 + display.getStrWidth("Vbat: ") + display.getStrWidth(String(telemetryData.DCVoltage).c_str()), Y1 + 32, "V");
                        break;
                    case NOTIFICATION:
                        display.setFont(u8g2_font_6x12_tf);
                        display.drawStr(X1 + 3, Y1 + 20, "Notification!");
                        break;
                    case SPEED:
                        display.setFont(u8g2_font_6x12_tf);
                        // draw speed: telemetryData.speed km/h
                        display.drawStr(X1 + 3, Y1 + 20, "Speed: ");
                        display.drawStr(X1 + 3 + display.getStrWidth("Speed: "), Y1 + 20, String(telemetryData.speed).c_str());
                        display.drawStr(X1 + 3 + display.getStrWidth("Speed: ") + display.getStrWidth(String(telemetryData.speed).c_str()) + 1, Y1 + 20, "km/h");
                        // next line is motor rpm
                        display.drawStr(X1 + 3, Y1 + 32, "RPM: ");
                        display.drawStr(X1 + 3 + display.getStrWidth("RPM: "), Y1 + 32, String(telemetryData.rpm).c_str());
                        // after the rpm, draw the furrent (right aligned)
                        display.drawStr(X2 - display.getStrWidth(String(telemetryData.DCCurrent).c_str()) - 10, Y1 + 32, String(telemetryData.DCCurrent).c_str());
                        display.drawStr(X2 - 8, Y1 + 32, "A");
                        break;
                    case READY:
                        display.setFont(u8g2_font_9x18_tf);
                        display.drawStr(X1 + (X2 - X1 - display.getStrWidth("Ready!")) / 2, Y1 + 27, "Ready!");
                        break;
                }
                display.sendBuffer();
            } else {
                // display.setPowerSave(1); // Turn off the display
                // clear and send the emty buffer
                display.clearBuffer();
                display.sendBuffer();
            }
            xSemaphoreGive(spiBusMutex);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void drawOdometer() {
    char buffer[20];  // Buffer to hold formatted strings

    display.setFont(u8g2_font_6x12_tf);

    // Draw odometer in the top left corner
    sprintf(buffer, "%d km", parameters[0].value);
    display.drawStr(X1 + 1, Y1 + 8, buffer);

    // Draw the trip odometer in the top right corner, right aligned
    uint32_t tripOdometer = getTripOdometer();
    sprintf(buffer, "%03d.%d km", tripOdometer / 10, tripOdometer % 10);  // Split value into whole and fractional parts
    int tripOdometerStrWidth = display.getStrWidth(buffer);
    display.drawStr(X2 - tripOdometerStrWidth - 2, Y1 + 8, buffer);

    // draw a line to section off the odometer
    display.drawLine(X1, Y1 + 9, X2 - 2, Y1 + 9);
}
