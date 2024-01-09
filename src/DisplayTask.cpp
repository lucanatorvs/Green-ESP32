#include "DisplayTask.h"
#include "Parameter.h"
#include "PinAssignments.h"
#include "PulseCounterTask.h"
#include "Semaphores.h"
#include "driveTelemetry.h"

#define X1 4    // x coordinate of the top left corner of the odometer
#define Y1 12   // y coordinate of the top left corner of the odometer
#define X2 123  // x coordinate of the bottom right corner of the odometer
#define Y2 50   // y coordinate of the bottom right corner of the odometer

U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI display(U8G2_R0, DISPLAY_CHIP_SELECT_PIN, DISPLAY_DATA_COMMAND_PIN, DISPLAY_RESET_PIN);

void displayTask(void * parameter);
void displayModeSwichTask(void * parameter);
void drawOdometer();

DisplayMode currentDisplayMode = EMPTY; // Global variable to keep track of the current display mode

void initializeDisplayTask() {
    if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
        display.begin();
        display.enableUTF8Print();

        xSemaphoreGive(spiBusMutex);
    }

    xTaskCreate(displayTask, "Display Task", 2048, NULL, 1, NULL);
    xTaskCreate(displayModeSwichTask, "Display Mode Switch Task", 2048, NULL, 1, NULL);
}

void displayModeSwichTask(void * parameter) {
    for (;;) {
        if (xSemaphoreTake(buttonStateSemaphore, portMAX_DELAY) == pdTRUE) {
            if (currentDisplayMode != OFF) {
                if (currentDisplayMode == NOTIFICATION || currentDisplayMode == READY) {
                    currentDisplayMode = EMPTY; // Dismiss the notification
                } else {
                    if (currentDisplayMode == EMPTY) {
                        currentDisplayMode = HELLO;
                    } else {
                        currentDisplayMode = EMPTY;
                    }
                }
            }
        }
    }
}

void displayTask(void * parameter) {
    for (;;) {
        if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
            if (currentDisplayMode != OFF) {
                display.setPowerSave(0); // Turn on the display
                display.enableUTF8Print();
                display.clearBuffer();
                // display.drawFrame(X1 - 1, Y1 - 1, X2 - X1 + 1, Y2 - Y1 + 1);
                drawOdometer();

                // Draw content based on the current display mode
                switch (currentDisplayMode) {
                    case EMPTY:
                        // Nothing to draw in this mode
                        break;
                    case HELLO:
                        display.setFont(u8g2_font_6x12_tf);
                        display.drawStr(X1 + 3, Y1 + 20, "Hello World!");
                        // dyaplay the time from milis in seconds
                        display.drawStr(X1 + 3, Y1 + 32, String(millis() / 1000).c_str());
                        break;
                    case NOTIFICATION:
                        display.setFont(u8g2_font_6x12_tf);
                        display.drawStr(X1 + 3, Y1 + 20, "Notification!");
                        break;
                    case READY:
                        display.setFont(u8g2_font_9x18_tf);
                        display.drawStr(X1 + (X2 - X1 - display.getStrWidth("Ready!")) / 2, Y1 + 27, "Ready!");
                        break;
                }
                display.sendBuffer();
            } else {
                display.setPowerSave(1); // Turn off the display
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
