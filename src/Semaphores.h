#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <Arduino.h>

extern SemaphoreHandle_t spiBusMutex;
extern SemaphoreHandle_t buttonSemaphore;
extern SemaphoreHandle_t buttonStateSemaphore;

void createSemaphores();

#endif // SEMAPHORES_H
