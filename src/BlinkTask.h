#ifndef BLINK_TASK_H
#define BLINK_TASK_H

#include <Arduino.h>

// Initialize blink task
void initializeBlinkTask();

// Blink task function
void blinkTask(void * parameter);

#endif // BLINK_TASK_H