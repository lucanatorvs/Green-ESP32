#include "BlinkTask.h"
#include "Parameter.h"

const int ledPin = LED_BUILTIN;

void initializeBlinkTask() {
    xTaskCreate(blinkTask, "Blink Task", 1024, NULL, 0, NULL);
}

void blinkTask(void * parameter) {
    pinMode(ledPin, OUTPUT);
    for (;;) {
        int blinkDelay = parameters[1].value;  // Get the blink speed from the new parameter
        digitalWrite(ledPin, HIGH);
        vTaskDelay(blinkDelay / portTICK_PERIOD_MS);
        digitalWrite(ledPin, LOW);
        vTaskDelay(blinkDelay / portTICK_PERIOD_MS);
    }
}
