#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <Arduino.h>

extern SemaphoreHandle_t spiBusMutex;

void createSemaphores();

#endif // SEMAPHORES_H