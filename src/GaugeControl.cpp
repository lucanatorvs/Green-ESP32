#include "GaugeControl.h"
#include "driveTelemetry.h"
#include "PulseCounterTask.h"

// Instantiate the HardwareSerial
HardwareSerial GaugeSerial(1);

// Define the ranges for each gauge
// minValue, maxValue, minAngle, maxAngle
GaugeRange SpeedometerRange     (0, 200, 105, 344);
GaugeRange TachometerRange      (0, 9000, 92, 335);
GaugeRange DynamometerRange     (-60, 90, 107, 235);
GaugeRange ChargeometerRange    (0, 100, 99, 191);
GaugeRange ThermometerRange     (-20, 100, 99, 194);

// Instantiate each gauge
Gauge Speedometer   ("Speedometer",     GaugeSerial, SpeedometerRange);
Gauge Tachometer    ("Tachometer",      GaugeSerial, TachometerRange);
Gauge Dynamometer   ("Dynamometer",     GaugeSerial, DynamometerRange);
Gauge Chargeometer  ("Chargeometer",    GaugeSerial, ChargeometerRange);
Gauge Thermometer   ("Thermometer",     GaugeSerial, ThermometerRange);

// variables
bool autoUpdate = true;

// semaphore
TaskHandle_t gaugeAnimatingTaskHandle = NULL;

// function prototypes
void gaugeControlTask(void * parameter);
void gaugeAnimatingTask(void * parameter);

void initializeGaugeControl() {
    // Initialize the serial communication for gauges
    GaugeSerial.begin(9600, SERIAL_8N1, GaugeRX, GaugeTX);

    // enable standby mode
    sendStandbyCommand(true);
    
    // set all gauges to 0 position
    Speedometer.setPosition(0);
    Tachometer.setPosition(0);
    Dynamometer.setPosition(-60);
    Chargeometer.setPosition(0);
    Thermometer.setPosition(-20);

    // start the gauge control task
    xTaskCreate(gaugeControlTask, "Gauge Control Task", 4096, NULL, 1, NULL);
}

void sendStandbyCommand(bool enable) {
    GaugeSerial.print("STBY:");
    GaugeSerial.print(enable ? '1' : '0');
    GaugeSerial.print('\n');
    if (enable) {
        if (autoUpdate == false) {
            if (gaugeAnimatingTaskHandle == NULL) {
                xTaskCreate(gaugeAnimatingTask, "Gauge Animating Task", 4096, NULL, 4, &gaugeAnimatingTaskHandle);
            }
        }
    } else {
        autoUpdate = false;

        // set all gauges to min position
        Speedometer.setPosition(Speedometer.getMinPosition());
        Tachometer.setPosition(Tachometer.getMinPosition());
        Dynamometer.setPosition(Dynamometer.getMinPosition());
        Chargeometer.setPosition(Chargeometer.getMinPosition());
        Thermometer.setPosition(Thermometer.getMinPosition());
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
            Tachometer.setPosition(abs(telemetryData.rpm));
            int Power = (telemetryData.DCCurrent * telemetryData.DCVoltage) / 1000; //KW
            Dynamometer.setPosition(Power);
            Chargeometer.setPosition(telemetryData.SoC);
            Speedometer.setPosition(telemetryData.speed);
            int8_t maxTempMotor = max(telemetryData.motorTemp, telemetryData.inverterTemp);
            int8_t maxTemp = max(maxTempMotor, telemetryData.BMSMaxModTemp);
            int gaugeTemp = max(maxTemp, telemetryData.BMSMaxCellTemp);
            if (telemetryData.BMSMinCellTemp <= 0) {
                gaugeTemp = telemetryData.BMSMinCellTemp;
            }
            Thermometer.setPosition(maxTemp);
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

    vTaskDelay(pdMS_TO_TICKS(300));

    int iMax = 30;
    for (int i = 0; i <= iMax; i++) {
        Chargeometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Chargeometer.getMinPosition(), Chargeometer.getMaxPosition()))));
        Speedometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Speedometer.getMinPosition(), Speedometer.getMaxPosition()))));
        Tachometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Tachometer.getMinPosition(), Tachometer.getMaxPosition()))));
        Dynamometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Dynamometer.getMinPosition(), Dynamometer.getMaxPosition()))));
        Thermometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Thermometer.getMinPosition(), Thermometer.getMaxPosition()))));
        vTaskDelay(pdMS_TO_TICKS(2));
    }

    // short pause
    vTaskDelay(pdMS_TO_TICKS(500));

    // reverse the animation
    for (int i = iMax; i >= 0; i--) {
        Thermometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Thermometer.getMinPosition(), Thermometer.getMaxPosition()))));
        Tachometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Tachometer.getMinPosition(), Tachometer.getMaxPosition()))));
        Dynamometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Dynamometer.getMinPosition(), Dynamometer.getMaxPosition()))));
        Speedometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Speedometer.getMinPosition(), Speedometer.getMaxPosition()))));
        Chargeometer.setPosition(static_cast<int>(ceil(map(i, 0, iMax, Chargeometer.getMinPosition(), Chargeometer.getMaxPosition()))));
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    autoUpdate = true;

    // Delete the task at the end of its execution
    gaugeAnimatingTaskHandle = NULL;
    vTaskDelete(NULL);
}
