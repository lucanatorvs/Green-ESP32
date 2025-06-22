#ifndef HELPER_TASKS_H
#define HELPER_TASKS_H

#include <Arduino.h>

// Initialize helper tasks
void initializeHelperTasks();

// Periodic function to be customized
void periodicFunction();

// Helper function to check if time has passed
bool hasTimePassed(unsigned long &lastTime, unsigned long interval);

// Lamp management functions
void manageLamps();
void setRunningLamp(bool state);

#endif // HELPER_TASKS_H