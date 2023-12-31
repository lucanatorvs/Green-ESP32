#include "CANListenerTask.h"
#include "PinAssignments.h"
#include "Semaphores.h"

namespace {
    MCP2515* mcp2515Ptr;
    SemaphoreHandle_t canMessageSemaphore;
    struct can_frame msg;
}

bool monitorCAN = false;
uint32_t filterCANID = 0;

void CanListenerTask(void * parameter);
void IRAM_ATTR OnCanReceive();
void LogCanMessage(const can_frame& msg);

void initializeCANListenerTask() {
    canMessageSemaphore = xSemaphoreCreateCounting(10, 0);

    if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
        MCP2515 mcp2515(MCP2515_CS_PIN);
        mcp2515.reset();
        mcp2515.setBitrate(CAN_500KBPS);
        mcp2515.setNormalMode();
        mcp2515Ptr = &mcp2515;
        xSemaphoreGive(spiBusMutex);
    }

    attachInterrupt(digitalPinToInterrupt(MCP2515_INT_PIN), OnCanReceive, FALLING);

    xTaskCreate(CanListenerTask, "CAN Listener Task", 2048, NULL, 3, NULL);
}

void CanListenerTask(void * parameter) {
    for (;;) {
        if (xSemaphoreTake(canMessageSemaphore, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
                auto readResult = mcp2515Ptr->readMessage(&msg);
                if (readResult == MCP2515::ERROR_OK) {
                    LogCanMessage(msg);
                } else {
                    // Handle error
                }
                xSemaphoreGive(spiBusMutex);
            }
        }
    }
}

void IRAM_ATTR OnCanReceive() {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(canMessageSemaphore, &higherPriorityTaskWoken);
    mcp2515Ptr->clearInterrupts();
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