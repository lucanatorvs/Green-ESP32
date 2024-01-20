#include <Arduino.h>
#include "CLI.h"
#include "Parameter.h"
#include "BlinkTask.h"
#include "DisplayTask.h"
#include "PulseCounterTask.h"
#include "CANListenerTask.h"
#include "Semaphores.h"
#include "GaugeControl.h"
#include "ButtonTask.h"
#include "Timers.h"

void setup() {
    // Initialize semaphores
    createSemaphores();

    // Initialize other components
    initializeParameter();
    initializeGaugeControl();

    // Initialize tasks
    initializeTimerTask();          // priority 3
    initializeCLI();                // priority 2
    initializeBlinkTask();          // priority 0
    initializeDisplayTask();        // priority 1
    initializePulseCounterTask();   // priority 2
    initializeCANListenerTask();    // priority 3
    initializeButtonTask();         // priority 4
}

void loop() {
    // Empty loop
    // with yield() function
    // to allow other tasks to run
    yield();
}
