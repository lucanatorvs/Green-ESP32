#include "Semaphores.h"

SemaphoreHandle_t spiBusMutex = NULL;
SemaphoreHandle_t buttonSemaphore = NULL;

void createSemaphores() {
    // Create the SPI bus mutex before starting tasks
    spiBusMutex = xSemaphoreCreateMutex();
    buttonSemaphore = xSemaphoreCreateBinary();
    if (spiBusMutex == NULL) {
        Serial.println("Failed to create SPI bus mutex");
        while (1);
    } else if (buttonSemaphore == NULL) {
        Serial.println("Failed to create button semaphore");
        while (1);
    }
}
