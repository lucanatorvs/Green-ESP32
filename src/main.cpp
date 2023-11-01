#include <Arduino.h>
#include "CLI.h"
#include "Parameter.h"
#include "BlinkTask.h"

void setup() {
    initializeCLI();
    initializeParameter();
    initializeBlinkTask();
}

void loop() {
    // Empty loop
}
