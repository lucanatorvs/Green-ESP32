#include "ButtonTask.h"
#include "Semaphores.h"
#include "PinAssignments.h"

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
    if ((currentTime - lastDebounceTime) > debounceDelay) {
        // Only toggle the LED if the new button state is HIGH (button released)
        if (digitalRead(BUTTON_PIN) == HIGH) {
            unsigned long pressDuration = currentTime - buttonPressTime;
            buttonPressed = false;
            xSemaphoreGiveFromISR(buttonSemaphore, NULL); // Use a semaphore to signal button release
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
            if (!buttonPressed) {
                // Button released
                unsigned long currentTime = millis();
                unsigned long pressDuration = currentTime - buttonPressTime;
                if (pressDuration < 750) {
                    Serial.println("Short press detected");
                } else {
                    Serial.println("Long press detected");
                }
                // increment semaphore count
                xSemaphoreGive(buttonStateSemaphore);
            }
        }
    }
}
