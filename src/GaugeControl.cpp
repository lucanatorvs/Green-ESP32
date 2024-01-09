#include "GaugeControl.h"
#include "driveTelemetry.h"

// Instantiate the HardwareSerial
HardwareSerial GaugeSerial(1);

// Define the ranges for each gauge
// minValue, maxValue, minAngle, maxAngle
GaugeRange SpeedometerRange     (0, 200, 17, 254);
GaugeRange TachometerRange      (0, 9000, 4, 246);
GaugeRange DynamometerRange     (-60, 90, 19, 144);
GaugeRange ChargeometerRange    (0, 100, 11, 99);
GaugeRange ThermometerRange     (-20, 100, 11, 103);

// Instantiate each gauge
Gauge Speedometer   ("Speedometer",     GaugeSerial, SpeedometerRange);
Gauge Tachometer    ("Tachometer",      GaugeSerial, TachometerRange);
Gauge Dynamometer   ("Dynamometer",     GaugeSerial, DynamometerRange);
Gauge Chargeometer  ("Chargeometer",    GaugeSerial, ChargeometerRange);
Gauge Thermometer   ("Thermometer",     GaugeSerial, ThermometerRange);

// variables
bool autoUpdate = false;

// semaphore
TaskHandle_t gaugeAnimatingTaskHandle = NULL;

// function prototypes
void gaugeControlTask(void * parameter);
void gaugeAnimatingTask(void * parameter);

void initializeGaugeControl() {
    // Initialize the serial communication for gauges
    GaugeSerial.begin(9600, SERIAL_8N1, GaugeRX, GaugeTX);

    // enable standby mode
    sendStandbyCommand(false);
    
    // set all gauges to 0 position
    Speedometer.setPosition(0);
    Tachometer.setPosition(0);
    Dynamometer.setPosition(0);
    Chargeometer.setPosition(0);
    Thermometer.setPosition(-20);

    // start the gauge control task
    xTaskCreate(gaugeControlTask, "Gauge Control Task", 2048, NULL, 1, NULL);
}

void sendStandbyCommand(bool enable) {
    GaugeSerial.print("STBY:");
    GaugeSerial.print(enable ? '1' : '0');
    GaugeSerial.print('\n');
    if (enable) {
        if (autoUpdate == false) {
            if (gaugeAnimatingTaskHandle == NULL) {
                xTaskCreate(gaugeAnimatingTask, "Gauge Animating Task", 2048, NULL, 4, &gaugeAnimatingTaskHandle);
            }
        }
    } else {
        autoUpdate = false;
    }
}

void enableAutoUpdate(bool enable) {
    autoUpdate = enable;
}

bool getAutoUpdate() {
    return autoUpdate;
}

void gaugeControlTask(void * parameter) {
    for (;;) {
        if (autoUpdate) {
            Tachometer.setPosition(telemetryData.rpm);
            int Power = telemetryData.DCCurrent * telemetryData.DCVoltage;
            Dynamometer.setPosition(Power);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void gaugeAnimatingTask(void * parameter) {
    // send the animation command
    autoUpdate = false;

    // set all gauges to min position
    Speedometer.setPosition(Speedometer.getMinPosition());
    Tachometer.setPosition(Tachometer.getMinPosition());
    Dynamometer.setPosition(Dynamometer.getMinPosition());
    Chargeometer.setPosition(Chargeometer.getMinPosition());
    Thermometer.setPosition(Thermometer.getMinPosition());

    vTaskDelay(pdMS_TO_TICKS(50));

    int iMax = 30;
    for (int i = 0; i <= iMax; i++) {
        Speedometer.setPosition(map (i, 0, iMax, Speedometer.getMinPosition(), Speedometer.getMaxPosition()));
        Tachometer.setPosition(map (i, 0, iMax, Tachometer.getMinPosition(), Tachometer.getMaxPosition()));
        Dynamometer.setPosition(map (i, 0, iMax, Dynamometer.getMinPosition(), Dynamometer.getMaxPosition()));
        Chargeometer.setPosition(map (i, 0, iMax, Chargeometer.getMinPosition(), Chargeometer.getMaxPosition()));
        Thermometer.setPosition(map (i, 0, iMax, Thermometer.getMinPosition(), Thermometer.getMaxPosition()));
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    // reverse the animation
    for (int i = iMax; i >= 0; i--) {
        Speedometer.setPosition(map (i, 0, iMax, Speedometer.getMinPosition(), Speedometer.getMaxPosition()));
        Tachometer.setPosition(map (i, 0, iMax, Tachometer.getMinPosition(), Tachometer.getMaxPosition()));
        Dynamometer.setPosition(map (i, 0, iMax, Dynamometer.getMinPosition(), Dynamometer.getMaxPosition()));
        Chargeometer.setPosition(map (i, 0, iMax, Chargeometer.getMinPosition(), Chargeometer.getMaxPosition()));
        Thermometer.setPosition(map (i, 0, iMax, Thermometer.getMinPosition(), Thermometer.getMaxPosition()));
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    autoUpdate = true;

    // Delete the task at the end of its execution
    gaugeAnimatingTaskHandle = NULL;
    vTaskDelete(NULL);
}
