// GaugeControl.h

#ifndef GAUGE_CONTROL_H
#define GAUGE_CONTROL_H

#include "PinAssignments.h"
#include <HardwareSerial.h>

// A helper class to manage range mappings for gauges
class GaugeRange {
private:
    int minValue;
    int maxValue;
    int minAngle;
    int maxAngle;

public:
    GaugeRange(int minVal, int maxVal, int minAng, int maxAng)
        : minValue(minVal), maxValue(maxVal), minAngle(minAng), maxAngle(maxAng) {}

    // Getters for min and max values for external use
    int getMinValue() const { return minValue; }
    int getMaxValue() const { return maxValue; }

    // Map a value to an angle based on the configured range
    int mapValueToAngle(int value) const {
        if (value < minValue) value = minValue;
        if (value > maxValue) value = maxValue;
        return minAngle + (maxAngle - minAngle) * (value - minValue) / (maxValue - minValue);
    }
};

class Gauge {
private:
    String name;
    int angle;
    HardwareSerial *serial;
    GaugeRange range;

public:
    Gauge(const String& gaugeName, HardwareSerial &serialRef, const GaugeRange& range)
        : name(gaugeName), angle(0), serial(&serialRef), range(range) {}

    void setPosition(int position) {
        angle = range.mapValueToAngle(position);
        sendCommand();
    }

    void sendCommand() {
        serial->print(name + ":" + angle);
        serial->print('\n');
    }
    
    // Additional functions to access the range for external use
    int getMinPosition() const {
        return range.getMinValue();
    }

    int getMaxPosition() const {
        return range.getMaxValue();
    }
};

// Declare the GaugeControl functions
void initializeGaugeControl();
void sendStandbyCommand(bool enable);

// Declare the gauges
extern Gauge Speedometer;
extern Gauge Tachometer;
extern Gauge Dynamometer;
extern Gauge Chargeometer;
extern Gauge Thermometer;

#endif // GAUGE_CONTROL_H
