#ifndef DisplayTask_h
#define DisplayTask_h

#include <Arduino.h>
#include <U8g2lib.h>
#include "Parameter.h"

// Initialize display task
void initializeDisplayTask();

enum DisplayMode {
    EMPTY,
    HELLO,
    SOC,
    SPEED,
    NOTIFICATION,
    READY,
    OFF
};

extern DisplayMode currentDisplayMode;

#endif // DisplayTask_h
