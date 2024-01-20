#include "ButtonTask.h"
#include "Semaphores.h"
#include "PinAssignments.h"
#include "PulseCounterTask.h"

// function prototypes
void buttonISR();
void ButtonTask(void * parameter);

volatile unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
const unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers
volatile bool buttonPressed = false;
volatile unsigned long buttonPressTime = 0;

void initializeButtonTask() {
    // Initialize button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, CHANGE);

    // start task
    xTaskCreate(ButtonTask, "Button Task", 2048, NULL, 4, NULL);
}

void IRAM_ATTR buttonISR() {
    unsigned long currentTime = millis();
    if ((currentTime - lastDebounceTime) > debounceDelay) { // Check if debounce period has passed
        if (digitalRead(BUTTON_PIN) == HIGH) {
            xSemaphoreGiveFromISR(buttonSemaphore, NULL);
        } else {
            buttonPressTime = currentTime;
            buttonPressed = true;
        }
        lastDebounceTime = currentTime;
    }
}

void ButtonTask(void * parameter) {
    for (;;) {
        if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE) {
            unsigned long currentTime = millis();
            unsigned long pressDuration = currentTime - buttonPressTime;
            if (buttonPressed && (pressDuration > debounceDelay)) {
                if (pressDuration < 750) {
                    xSemaphoreGive(buttonStateSemaphore);
                } else {
                    resetTripOdometer();
                }
                buttonPressed = false;
            }
        }
    }
}
