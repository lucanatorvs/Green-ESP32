#ifndef CAN_LISTENER_TASK_H
#define CAN_LISTENER_TASK_H

#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

void initializeCANListenerTask();

extern bool monitorCAN;
extern uint32_t filterCANID;

void setCANMonitoring(bool state, uint32_t filterID = 0);
int CANMonitoring();

#endif // CAN_LISTENER_TASK_H
