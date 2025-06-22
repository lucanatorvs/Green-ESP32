#ifndef DisplayTask_h
#define DisplayTask_h

#include <Arduino.h>
#include <U8g2lib.h>
#include "Parameter.h"

// Initialize display task
void initializeDisplayTask();

// Ignition control functions
void setIgnitionOverride(bool enabled);
bool getIgnitionOverride();
void setIgnitionState(bool on);
bool getIgnitionState();

// Keep this list updated with CLI.cpp, and DisplayTask.cpp
enum DisplayMode {
    EMPTY,
    START,
    SOC,
    SPEED,
    NOTIFICATION,
    READY,
    OFF
};

extern DisplayMode currentDisplayMode;

#endif // DisplayTask_h
