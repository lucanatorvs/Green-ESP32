#include "Semaphores.h"

SemaphoreHandle_t spiBusMutex = NULL;
SemaphoreHandle_t buttonSemaphore = NULL;
SemaphoreHandle_t buttonStateSemaphore = NULL;

void createSemaphores() {
    // Create the SPI bus mutex before starting tasks
    spiBusMutex = xSemaphoreCreateMutex(); // this mutex is no longer needed, since the SPI bus is now only used in the display task
    buttonSemaphore = xSemaphoreCreateBinary();
    buttonStateSemaphore = xSemaphoreCreateCounting(2, 0);
    if (spiBusMutex == NULL) {
        Serial.println("Failed to create SPI bus mutex");
        while (1);
    } else if (buttonSemaphore == NULL) {
        Serial.println("Failed to create button semaphore");
        while (1);
    } else if (buttonStateSemaphore == NULL) {
        Serial.println("Failed to create button state semaphore");
        while (1);
    }
}
