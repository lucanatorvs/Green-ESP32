#include "PulseCounterTask.h"
#include "Parameter.h"
#include <driver/pcnt.h>

#define PULSE_INPUT_PIN 26
#define PULSE_COUNTER_UNIT PCNT_UNIT_0
#define PCNT_H_LIM_VAL 10000
#define PULSEFILTER 34464
#define SAMPLE_SIZE 10

volatile uint32_t frequency_buffer[SAMPLE_SIZE] = {0};
volatile int frequency_buffer_index = 0;
volatile uint32_t speed = 0;  // Variable to store the speed value

// Pulse Counter configuration
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

void initializePulseCounterTask() {
    // Pulse Counter configuration
    pcnt_unit_config(&pcnt_config);
    pcnt_set_filter_value(PULSE_COUNTER_UNIT, PULSEFILTER);
    pcnt_filter_enable(PULSE_COUNTER_UNIT);

    xTaskCreate(calculate_speed_task, "Calculate Speed", 1000, NULL, 3, NULL);
}

void calculate_speed_task(void *pvParameters) {
    while (1) {
        int PulseDelay = parameters[2].value;
        int16_t count;
        pcnt_get_counter_value(PULSE_COUNTER_UNIT, &count);
        uint32_t local_frequency = count * 1000 / PulseDelay;
        frequency_buffer[frequency_buffer_index] = local_frequency;
        frequency_buffer_index = (frequency_buffer_index + 1) % SAMPLE_SIZE;  // Make sure index stays within buffer size
        
        // Placeholder calculation for speed (you can replace this with a more meaningful calculation)
        uint32_t total_frequency = 0;
        for (int i = 0; i < SAMPLE_SIZE; i++) {
            total_frequency += frequency_buffer[i];
        }
        speed = total_frequency / SAMPLE_SIZE;  // Average frequency as speed
        
        pcnt_counter_clear(PULSE_COUNTER_UNIT);
        vTaskDelay(PulseDelay / portTICK_PERIOD_MS);
    }
}

uint32_t getSpeed() {
    return speed;
}