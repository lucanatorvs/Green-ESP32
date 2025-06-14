#include "CANListenerTask.h"
#include "PinAssignments.h"
#include "Semaphores.h"
#include "driveTelemetry.h"
#include "Timers.h"
#include "DisplayTask.h"
#include "GaugeControl.h"

Telemetry telemetryData;
bool monitorCAN = false;
uint32_t filterCANID = 0;
CanFrame rxFrame;

// Helper function to send a CAN frame
void sendCANFrame(uint32_t identifier, bool extd, uint8_t dlc, uint8_t* data) {
    CanFrame txFrame = {0};
    txFrame.identifier = identifier;
    txFrame.extd = extd;
    txFrame.data_length_code = dlc;
    
    for(int i = 0; i < dlc && i < 8; i++) {
        txFrame.data[i] = data[i];
    }
    
    ESP32Can.writeFrame(txFrame);
}

// Forward declarations
void CanListenerTask(void * parameter);
void LogCanMessage();
void HandleCanMessage();
void onMotorOff();
void onMotorON();

Timer motorTimer(3200, onMotorOff); // 3200 milliseconds timeout

void initializeCANListenerTask() {

    // Initialize the CAN controller at 250 kbps
    if(ESP32Can.begin(ESP32Can.convertSpeed(250), CAN_TX_PIN, CAN_RX_PIN, 10, 10)) {
        Serial.println("CAN bus started!");
    } else {
        Serial.println("Starting CAN failed!");
        return;
    }

    xTaskCreate(CanListenerTask, "CAN Listener Task", 4096 * 8, NULL, 1, NULL);
}

void CanListenerTask(void * parameter) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1)); // 100Hz polling rate

        // Try to read a CAN frame with a timeout of 1ms
        if(ESP32Can.readFrame(rxFrame, 1)) {
            LogCanMessage();
            HandleCanMessage();
        }
    }
}

void LogCanMessage() {
    if (monitorCAN && (filterCANID == 0 || rxFrame.identifier == filterCANID)) {
        Serial.print("ID: ");
        Serial.print(rxFrame.identifier, HEX);
        Serial.print(" DLC: ");
        Serial.print(rxFrame.data_length_code);
        Serial.print(" Data: ");
        for (int i = 0; i < rxFrame.data_length_code; i++) {
            Serial.print(rxFrame.data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void setCANMonitoring(bool state, uint32_t filterID) {
    monitorCAN = state;
    filterCANID = filterID;
}

int CANMonitoring() {
    return monitorCAN;
}

void onMotorOff() {
    // Implement what happens when the motor is considered "off"
    Serial.println("Motor is off");
    // if the mode is ready, chinge it to empty, otherwise, do nothing
    if (currentDisplayMode == READY) {
        currentDisplayMode = EMPTY;
    }
}

void onMotorON() {
    // Implement what happens when the motor is considered "on"
    Serial.println("Motor is on");
    currentDisplayMode = READY;
}

void HandleCanMessage() {
    uint32_t canId = rxFrame.identifier;
    int dlc = rxFrame.data_length_code;
    uint8_t* msgData = rxFrame.data;

    if (canId == 0x06) {
        // check if the message is valid
        bool ignore = false;
        for (int i = 0; i < dlc; i++) {
            if (msgData[i] == 0xFF) {
                ignore = true;
                break;
            }
        }
        if (ignore) return;

        // Motor temperature: (0 to 255) - 40 [C]
        telemetryData.motorTemp = msgData[0] - 40;
        // Inverter temperature: (0 to 255) - 40 [C]
        telemetryData.inverterTemp = msgData[1] - 40;
        // Motor RPM: (0 to 65535) [RPM]
        telemetryData.rpm = (msgData[3] << 8) + msgData[2];
        // Motor (DC) voltage: (0 to 65535) / 10 [V]
        telemetryData.DCVoltage = ((msgData[5] << 8) + msgData[4]) / 10.0;
        // Motor (DC) current: (0 to 65535) / 10 [A]
        telemetryData.DCCurrent = (int16_t)((msgData[7] << 8) + msgData[6]) / 10.0;

    } else if (canId == 0x07) {
        // Motor is "on". Start or reset the timer.
        if (!motorTimer.isRunning()) {
            motorTimer.start();
            onMotorON();
        } else {
            motorTimer.reset();
        }
        telemetryData.powerUnitFlags = (msgData[1] << 8) + msgData[0];

    } else if ((canId & 0xFFFF0000) == 0x19B50000) {
        // Handel EMUS messages (0x19B5xxxx)
        uint16_t canAddr = canId & 0xFFFF;
        switch (canAddr) {
            case 0x0000:
                // Handle 0x99B50000 Overall Parameters
                telemetryData.BMSInputSignalFlags = msgData[0];
                telemetryData.BMSOutputSignalFlags = msgData[1];
                telemetryData.BMSNumberOfCells = (msgData[2] << 8) + msgData[7];
                telemetryData.BMSChargingState = msgData[3];
                telemetryData.BMSCsDuration = (msgData[4] << 8) + msgData[5];
                telemetryData.BMSLastChargingError = msgData[6];
                break;
            case 0x0007:
                // Handle 0x99B50007 Diagnostics Codes
                telemetryData.BMSProtectionFlags = (msgData[3] << 24) + (msgData[2] << 16) + (msgData[1] << 8) + msgData[0];
                telemetryData.BMSReductionFlags = msgData[4];
                telemetryData.BMSBatteryStatusFlags = msgData[7];
                break;
            case 0x0002:
                // Handle 0x99B50002 Cell Module Temperature Overall Parameters
                telemetryData.BMSMinModTemp = msgData[0] - 100; // Convert to Celsius
                telemetryData.BMSMaxModTemp = msgData[1] - 100; // Convert to Celsius
                telemetryData.BMSAverageModTemp = msgData[2] - 100; // Convert to Celsius
                break;
            case 0x0008:
                // Handle 0x99B50008 Cell Temperature Overall Parameters
                telemetryData.BMSMinCellTemp = msgData[0] - 100; // Convert to Celsius
                telemetryData.BMSMaxCellTemp = msgData[1] - 100; // Convert to Celsius
                telemetryData.BMSAverageCellTemp = msgData[2] - 100; // Convert to Celsius
                break;
            case 0x0500:
                // Handle 0x99B50500 State of Charge parameters
                telemetryData.Current = (msgData[0] << 8) + msgData[1];
                telemetryData.Charge = (msgData[2] << 8) + msgData[3];
                telemetryData.SoC = (msgData[5] << 8) + msgData[6];
                break;
            default:
                break;
        }
    }
}