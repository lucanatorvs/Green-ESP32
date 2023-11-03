#ifndef PULSE_COUNTER_TASK_H
#define PULSE_COUNTER_TASK_H

#include <Arduino.h>

// Function to initialize the Pulse Counter Task
void initializePulseCounterTask();

// Function to get the speed value
uint32_t getSpeed();

// Function for the trip odometer
uint32_t getTripOdometer();
void setTripOdometer(uint32_t value);
void resetTripOdometer();

#endif  // PULSE_COUNTER_TASK_H
