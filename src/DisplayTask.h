#ifndef DisplayTask_h
#define DisplayTask_h

#include <Arduino.h>
#include <U8g2lib.h>
#include "Parameter.h"

// Initialize display task
void initializeDisplayTask();

// Display task function
void displayTask(void * parameter);

#endif
