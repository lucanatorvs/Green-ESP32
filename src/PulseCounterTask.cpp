#include "PulseCounterTask.h"
#include "Parameter.h"
#include "PinAssignments.h"
#include <driver/pcnt.h>
#include "driveTelemetry.h"

volatile uint32_t pulseCount = 0;
volatile uint32_t speed = 0;
volatile uint32_t accumulated_distance = 0;
volatile uint32_t trip_distance = 0;

void calculate_speed_task(void *pvParameters);
void pulse_reading_task(void *pvParameters);
void checkAndIncrementOdometer();
void checkAndResetTripOdometer();

void initializePulseCounterTask() {
    pinMode(PULSE_INPUT_PIN, INPUT);

    xTaskCreate(pulse_reading_task, "PulseReadingTask", 2048, NULL, 1, NULL);
    xTaskCreate(calculate_speed_task, "CalculateSpeedTask", 2048, NULL, 2, NULL);
}

void pulse_reading_task(void *pvParameters) {
    bool lastState = digitalRead(PULSE_INPUT_PIN);
    for (;;) {
        bool currentState = digitalRead(PULSE_INPUT_PIN);
        if (currentState != lastState) {
            pulseCount++;
            lastState = currentState;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void calculate_speed_task(void *pvParameters) {
    TickType_t lastTime = xTaskGetTickCount(); // Initial time in ticks
    float alpha = 0.1; // Smoothing factor for EMA, adjust as needed
    float smoothedPulseCount = 0; // EMA of pulse count

    for (;;) {
        int PulseDistance = parameters[3].value; // Distance in mm per pulse

        // Update the smoothed pulse count with an exponential moving average
        smoothedPulseCount = alpha * pulseCount + (1 - alpha) * smoothedPulseCount;
        pulseCount = 0; // Reset the pulse count for the next period

        uint32_t distance = smoothedPulseCount * PulseDistance; // distance in mm

        accumulated_distance += distance;
        trip_distance += distance;

        checkAndIncrementOdometer();
        checkAndResetTripOdometer();

        TickType_t currentTime = xTaskGetTickCount();
        TickType_t elapsedTime = currentTime - lastTime; // Elapsed time in ticks
        lastTime = currentTime; // Update lastTime for the next cycle

        // Convert elapsed time from ticks to milliseconds
        uint32_t elapsedTimeMs = (elapsedTime * 1000) / configTICK_RATE_HZ;

        // Calculate speed
        uint32_t local = 0;
        if (elapsedTimeMs > 0) {
            local = distance * 1000 / elapsedTimeMs; // speed in mm/s
        }
        speed = local * 36 / 10000; // speed in km/h

        telemetryData.speed = speed;

        vTaskDelay(pdMS_TO_TICKS(parameters[2].value));
    }
}

uint32_t getSpeed() {
    return speed;
}

void checkAndIncrementOdometer() {
    if (accumulated_distance >= 1000000) {
        int OdometerCount = parameters[0].value;
        OdometerCount += 1;
        parameters[0].value = OdometerCount;
        storeParametersToNVS(0);
        accumulated_distance -= 1000000;
    }
}

void checkAndResetTripOdometer() {
    if (trip_distance >= 1000000000) { // 1000000000 mm = 100000 m = 100 km
        trip_distance -= 1000000000;
    }
}

uint32_t getTripOdometer() {
    return trip_distance / 100000; // convert mm to 100 m
}

void resetTripOdometer() {
    trip_distance = 0;
}

void setTripOdometer(uint32_t value) {
    trip_distance = value * 100000; // convert 100 m to mm
}
