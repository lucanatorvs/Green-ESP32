#include <Arduino.h>
#include "CLI.h"
#include "Parameter.h"
#include "BlinkTask.h"
#include "DisplayTask.h"
#include "PulseCounterTask.h"
#include "CANListenerTask.h"
#include "Semaphores.h"

void setup() {
    createSemaphores();

    initializeCLI();                // priority 2
    initializeParameter();
    initializeBlinkTask();          // priority 0
    initializeDisplayTask();        // priority 1
    initializePulseCounterTask();   // priority 3
    initializeCANListenerTask();    // priority 2
}

void loop() {
    // Empty loop
}
