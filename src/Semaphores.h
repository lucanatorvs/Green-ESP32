#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <Arduino.h>

extern SemaphoreHandle_t spiBusMutex; // Not used since switching to build in CAN Controller,
                                      // but kept for compatibility with the display task
extern SemaphoreHandle_t buttonSemaphore;
extern SemaphoreHandle_t buttonStateSemaphore;

void createSemaphores();

#endif // SEMAPHORES_H
