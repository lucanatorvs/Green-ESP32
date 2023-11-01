#include <Arduino.h>
#include "CLI.h"
#include "Parameter.h"
#include "BlinkTask.h"
#include "DisplayTask.h"

void setup() {
    initializeCLI();            // priority 2
    initializeParameter();
    initializeBlinkTask();      // priority 0
    initializeDisplayTask();    // priority 1
}

void loop() {
    // Empty loop
}
