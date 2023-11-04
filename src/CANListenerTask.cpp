#include "CANListenerTask.h"
#include "PinAssignments.h"
#include "Semaphores.h"

MCP2515* mcp2515Ptr;

void canListenerTask(void * parameter);
void IRAM_ATTR onCANReceive();

void initializeCANListenerTask() {
    // MCP2515 mcp2515(MCP2515_CS_PIN);
    // mcp2515.reset();
    // mcp2515.setBitrate(CAN_500KBPS);  // Set the CAN bitrate to 500 kbps
    // mcp2515.setNormalMode();  // Set the MCP2515 to normal mode
    // attachInterrupt(digitalPinToInterrupt(MCP2515_INT_PIN), onCANReceive, FALLING);  // Set up the interrupt
    // mcp2515Ptr = &mcp2515;
    // xTaskCreate(canListenerTask, "CAN Listener Task", 2048, NULL, 1, NULL);
}

void canListenerTask(void * parameter) {
    for (;;) {
        // struct can_frame msg;
        // if (mcp2515Ptr->readMessage(&msg) == MCP2515::ERROR_OK) {
        //     // Process the received CAN message here
        // }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Interrupt service routine
void IRAM_ATTR onCANReceive() {
    // Indicate that a CAN message is ready to be read
    // This could be done by setting a flag, or directly reading the message
}
