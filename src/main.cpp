#include <Arduino.h>
#include "CLI.h"
#include "Parameter.h"
#include "BlinkTask.h"
#include "DisplayTask.h"
#include "PulseCounterTask.h"
#include "CANListenerTask.h"
#include "Semaphores.h"
#include "GaugeControl.h"

void setup() {
    // Initialize semaphores
    createSemaphores();

    // Initialize other components
    initializeParameter();
    initializeGaugeControl();

    // Initialize tasks
    initializeCLI();                // priority 2
    initializeBlinkTask();          // priority 0
    initializeDisplayTask();        // priority 1
    initializePulseCounterTask();   // priority 2
    initializeCANListenerTask();    // priority 3
}

void loop() {
    // Empty loop
}
