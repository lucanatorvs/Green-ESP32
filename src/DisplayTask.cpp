#include "DisplayTask.h"
#include "Parameter.h"  // Include the source file for your parameters

#define DISPLAY_CHIP_SELECT_PIN 21
#define DISPLAY_DATA_COMMAND_PIN 17
#define DISPLAY_RESET_PIN 16

U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI display(U8G2_R2, DISPLAY_CHIP_SELECT_PIN, DISPLAY_DATA_COMMAND_PIN, DISPLAY_RESET_PIN);
SemaphoreHandle_t display_semaphore;

void initializeDisplayTask() {
    display.begin();
    display.enableUTF8Print();

    display_semaphore = xSemaphoreCreateMutex();

    xTaskCreate(displayTask, "Display Task", 2048, NULL, 1, NULL);
}

void displayTask(void * parameter) {
    for (;;) {
        if (xSemaphoreTake(display_semaphore, (TickType_t)10) == pdTRUE) {
            display.clearBuffer();
            
            display.setFont(u8g2_font_04b_03_tr);
            
            for (int i = 0; i < numParameters; i++) {
                String paramStr = String(parameters[i].name) + ": " + String(parameters[i].value);
                display.drawStr(0, (i + 1) * 8, paramStr.c_str());
            }
            
            display.sendBuffer();
            
            xSemaphoreGive(display_semaphore);
            
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
