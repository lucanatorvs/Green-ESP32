#include "Semaphores.h"

SemaphoreHandle_t spiBusMutex = NULL;

void createSemaphores() {
    // Create the SPI bus mutex before starting tasks
    spiBusMutex = xSemaphoreCreateMutex();
    if (spiBusMutex == NULL) {
        Serial.println("Failed to create SPI bus mutex");
        while (1);
    }
}