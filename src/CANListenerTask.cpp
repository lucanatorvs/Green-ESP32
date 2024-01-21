#include "CANListenerTask.h"
#include "PinAssignments.h"
#include "Semaphores.h"
#include "driveTelemetry.h"
#include "Timers.h"
#include "DisplayTask.h"
#include "GaugeControl.h"

namespace {
    struct can_frame msg;
    SemaphoreHandle_t canMessageSemaphore;
}

MCP2515 mcp2515(MCP2515_CS_PIN);  // Global MCP2515 object
Telemetry telemetryData;
bool monitorCAN = false;
uint32_t filterCANID = 0;

// Forward declarations
void CanListenerTask(void * parameter);
void IRAM_ATTR OnCanReceive();
void LogCanMessage(const can_frame& msg);
void HandleCanMessage(const can_frame& msg);
void onMotorOff();
void onMotorON();

Timer motorTimer(4000, onMotorOff); // 1200 milliseconds timeout

void initializeCANListenerTask() {
    canMessageSemaphore = xSemaphoreCreateCounting(10, 0);

    if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
        mcp2515.reset();
        mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);
        mcp2515.setNormalMode();
        // enable both receive buffers to receive messages and interrupt on receive
        xSemaphoreGive(spiBusMutex);
    }

    attachInterrupt(digitalPinToInterrupt(MCP2515_INT_PIN), OnCanReceive, FALLING);
    pinMode(MCP2515_INT_PIN, INPUT_PULLUP);

    xTaskCreate(CanListenerTask, "CAN Listener Task", 2048, NULL, 1, NULL);
}

void CanListenerTask(void * parameter) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5)); // 500Hz polling rate
        if (xSemaphoreTake(canMessageSemaphore, portMAX_DELAY) == pdTRUE) {
            mcp2515.clearInterrupts();
            if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
                // enter critical section
                void taskENTER_CRITICAL( void );

                auto readResult = mcp2515.readMessage(&msg);

                // exit critical section
                void taskEXIT_CRITICAL( void );

                xSemaphoreGive(spiBusMutex);
                if (readResult == MCP2515::ERROR_OK) {
                    LogCanMessage(msg);
                    HandleCanMessage(msg);
                } else {
                    // Handle error
                    // Serial.print("Error: ");
                    // Serial.println(readResult);
                }
            }
        }
    }
}

void IRAM_ATTR OnCanReceive() {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(canMessageSemaphore, &higherPriorityTaskWoken);
    if (higherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void LogCanMessage(const can_frame& msg) {
    if (monitorCAN && (filterCANID == 0 || msg.can_id == filterCANID)) {
        Serial.print("ID: ");
        Serial.print(msg.can_id, HEX);
        Serial.print(" DLC: ");
        Serial.print(msg.can_dlc);
        Serial.print(" Data: ");
        for (int i = 0; i < msg.can_dlc; i++) {
            Serial.print(msg.data[i], HEX);
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
    currentDisplayMode = OFF;
    sendStandbyCommand(false);
}

void onMotorON() {
    // Implement what happens when the motor is considered "on"
    Serial.println("Motor is on");
    currentDisplayMode = READY;
    sendStandbyCommand(true);
}

void HandleCanMessage(const can_frame& msg) {
    if (msg.can_id == 0x06) {
        // Motor temperature: (0 to 255) - 40 [C]
        telemetryData.motorTemp = msg.data[0] - 40;
        // Inverter temperature: (0 to 255) - 40 [C]
        telemetryData.inverterTemp = msg.data[1] - 40;
        // Motor RPM: (0 to 65535) [RPM]
        telemetryData.rpm = (msg.data[3] << 8) + msg.data[2];
        // Motor (DC) voltage: (0 to 65535) / 10 [V]
        telemetryData.DCVoltage = ((msg.data[5] << 8) + msg.data[4]) / 10.0;
        // Motor (DC) current: (0 to 65535) / 10 [A]
        telemetryData.DCCurrent = (int16_t)((msg.data[7] << 8) + msg.data[6]) / 10.0;
    } else if (msg.can_id == 0x07) {
        // Motor is "on". Start or reset the timer.
        if (!motorTimer.isRunning()) {
            motorTimer.start();
            onMotorON();
        } else {
            motorTimer.reset();
        }
        telemetryData.powerUnitFlags = (msg.data[1] << 8) + msg.data[0];
    } else if (msg.can_id == 0x99B50500) {
        telemetryData.Current = (msg.data[0] << 8) + msg.data[1];
        telemetryData.Charge = (msg.data[2] << 8) + msg.data[3];
        telemetryData.SoC = msg.data[6];
    }
}
