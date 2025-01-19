#ifndef PIN_ASSIGNMENTS_H
#define PIN_ASSIGNMENTS_H

// Display
#define DISPLAY_CHIP_SELECT_PIN 21      // IO21
#define DISPLAY_DATA_COMMAND_PIN 17     // IO17 (TX)
#define DISPLAY_RESET_PIN 16            // IO16 (RX)

// Pulse input
#define PULSE_INPUT_PIN 4               // A5

// CAN bus
#define CAN_TX_PIN 14                        // IO14
#define CAN_RX_PIN 32                        // IO32

// Gauge
#define GaugeRX 27                      // IO27
#define GaugeTX 12                      // IO12

// Button
#define BUTTONPIN 33                   // IO33

// Buzzer
#define BUZZER_PIN 14                   // IO14

// door close sensor
#define DOOR_CLOSE_SENSOR_PIN 39        // A3

// ignition switch
#define IGNITION_SWITCH_PIN 36          // A4

// Brake light output
#define BRAKE_LIGHT_PIN 15              // IO15

// Lamp output
#define SOC_Lamp_PIN 34                 // A3
#define TEMPERATURE_Lamp_PIN 4          // A5
#define BATTERY_Lamp_PIN 23             // IO23 (SDA)
#define RUNNIG_Lamp_PIN 22              // IO22 (SCL)

#endif // PIN_ASSIGNMENTS_H
