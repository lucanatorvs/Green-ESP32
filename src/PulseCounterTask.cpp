#include "PulseCounterTask.h"
#include "Parameter.h"
#include "PinAssignments.h"
#include <driver/pcnt.h>


#define PULSE_COUNTER_UNIT PCNT_UNIT_0
#define PCNT_H_LIM_VAL 10000
#define PULSEFILTER 34464
#define SAMPLE_SIZE 10

volatile uint32_t frequency_buffer[SAMPLE_SIZE] = {0};
volatile int frequency_buffer_index = 0;
volatile uint32_t speed = 0;
volatile uint32_t accumulated_distance = 0;

pcnt_config_t pcnt_config = {
    .pulse_gpio_num = PULSE_INPUT_PIN,
    .ctrl_gpio_num = PCNT_PIN_NOT_USED,
    .lctrl_mode = PCNT_MODE_KEEP,
    .hctrl_mode = PCNT_MODE_KEEP,
    .pos_mode = PCNT_COUNT_INC,
    .neg_mode = PCNT_COUNT_DIS,
    .counter_h_lim = PCNT_H_LIM_VAL,
    .counter_l_lim = 0,
    .unit = PULSE_COUNTER_UNIT,
    .channel = PCNT_CHANNEL_0,
};

void calculate_speed_task(void *pvParameters);
void checkAndIncrementOdometer();

void initializePulseCounterTask() {
    pcnt_unit_config(&pcnt_config);
    pcnt_set_filter_value(PULSE_COUNTER_UNIT, PULSEFILTER);
    pcnt_filter_enable(PULSE_COUNTER_UNIT);

    xTaskCreate(calculate_speed_task, "Calculate Speed", 1000, NULL, 3, NULL);
}

void calculate_speed_task(void *pvParameters) {
    for (;;) {
        int PulseDelay = parameters[2].value;
        int PulseDistance = parameters[3].value; // Distance in mm per pulse
        int16_t count;
        pcnt_get_counter_value(PULSE_COUNTER_UNIT, &count);

        accumulated_distance += count * PulseDistance;

        checkAndIncrementOdometer();

        // I could optimise this by precmputing the speed factor and using that instead of the division.
        // At the cost of being able to change the speed factor (PulseDelay) at runtime
        uint32_t local = count * PulseDistance; // distance in mm
        local = local * 1000 / PulseDelay; // speed in mm/s
        speed = local * 36 / 100000; // speed in km/h

        pcnt_counter_clear(PULSE_COUNTER_UNIT);
        vTaskDelay(PulseDelay / portTICK_PERIOD_MS);
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
