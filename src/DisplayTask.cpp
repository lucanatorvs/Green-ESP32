#include "DisplayTask.h"
#include "Parameter.h"
#include "PinAssignments.h"
#include "PulseCounterTask.h"
#include "Semaphores.h"

#define X1 5    // x coordinate of the top left corner of the odometer
#define Y1 10   // y coordinate of the top left corner of the odometer
#define X2 120  // x coordinate of the bottom right corner of the odometer
#define Y2 60   // y coordinate of the bottom right corner of the odometer

U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI display(U8G2_R2, DISPLAY_CHIP_SELECT_PIN, DISPLAY_DATA_COMMAND_PIN, DISPLAY_RESET_PIN);

void displayTask(void * parameter);
void drawOdometer();

void initializeDisplayTask() {
    if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
        display.begin();
        display.enableUTF8Print();

        xSemaphoreGive(spiBusMutex);
    }

    xTaskCreate(displayTask, "Display Task", 2048, NULL, 1, NULL);
}

void displayTask(void * parameter) {
    for (;;) {
        if (xSemaphoreTake(spiBusMutex, portMAX_DELAY)) {
            display.enableUTF8Print();

            display.clearBuffer();

            // draw a frame from xy1 to xy2, just outside the visible area, use this to position the display
            display.drawFrame(X1 - 1, Y1 - 1, X2 - X1 + 1, Y2 - Y1 + 1);

            drawOdometer();


            display.sendBuffer();

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
