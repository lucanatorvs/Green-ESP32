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

// Forward declarations
void CanListenerTask(void * parameter);
void LogCanMessage();
void HandleCanMessage();
void onMotorOff();
void onMotorON();

Timer motorTimer(3200, onMotorOff); // 3200 milliseconds timeout

void initializeCANListenerTask() {
    // Set the pins for the built-in CAN controller
    CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);  // Use your defined RX and TX pins

    // Initialize the built-in CAN controller at 250 kbps
    if (!CAN.begin(250E3)) {
        Serial.println("Starting CAN failed!");
        return;
    }

    xTaskCreate(CanListenerTask, "CAN Listener Task", 2048, NULL, 2, NULL);
}

void CanListenerTask(void * parameter) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10)); // 100Hz polling rate

        int packetSize = CAN.parsePacket();

        if (packetSize) {
            LogCanMessage();
            HandleCanMessage();
        }
    }
}

void LogCanMessage() {
    if (monitorCAN && (filterCANID == 0 || CAN.packetId() == filterCANID)) {
        Serial.print("ID: ");
        Serial.print(CAN.packetId(), HEX);
        Serial.print(" DLC: ");
        Serial.print(CAN.packetDlc());
        Serial.print(" Data: ");
        while (CAN.available()) {
            uint8_t dataByte = CAN.read();
            Serial.print(dataByte, HEX);
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
    uint32_t canId = CAN.packetId();
    int dlc = CAN.packetDlc();

    uint8_t msgData[8];
    int index = 0;
    while (CAN.available() && index < 8) {
        msgData[index++] = CAN.read();
    }

    if (canId == 0x06) {
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
    } else if (canId == 0x99B50500) {
        telemetryData.Current = (msgData[0] << 8) + msgData[1];
        telemetryData.Charge = (msgData[2] << 8) + msgData[3];
        telemetryData.SoC = msgData[6];
    }
}